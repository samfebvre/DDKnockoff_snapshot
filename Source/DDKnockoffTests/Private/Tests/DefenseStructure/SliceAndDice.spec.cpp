#include "CoreMinimal.h"
#include "Core/ManagerHandlerSubsystem.h"
#include "Debug/DebugInformationManager.h"
#include "Entities/EntityManager.h"
#include "Tests/Common/BaseSpec.h"
#include "Structures/SliceAndDice/SliceAndDice.h"
#include "Structures/SliceAndDice/SliceAndDiceEnums.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/DefenseStructure/DefenseStructureTestHelpers.h"
#include "Tests/DefenseStructure/DefensiveStructure_CommonTests.h"
#include "Mocks/MockEnemy.h"
#include "Structures/StructurePlacementManager.h"
#include "Structures/Components/EnemyDetectionComponent.h"

BEGIN_DEFINE_SPEC(FSliceAndDiceSpec,
                  "DDKnockoff.Structures.SliceAndDice",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |
                  EAutomationTestFlags::ProductFilter)

    // SPEC_BOILERPLATE_BEGIN
    BaseSpec BaseSpec;
    // SPEC_BOILERPLATE_END
    
    TObjectPtr<ASliceAndDice> TestSliceAndDice;
END_DEFINE_SPEC(FSliceAndDiceSpec)

void FSliceAndDiceSpec::Define() {
    BeforeEach([this] {
        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.SetupBaseSpecEnvironment({UDebugInformationManager::StaticClass(),
                                           UEntityManager::StaticClass(),
                                           UStructurePlacementManager::StaticClass()});
        // SPEC_BOILERPLATE_END

        UClass* LoadedClass = FDefenseStructureTestHelpers::LoadDefenseStructureBlueprintClass(ASliceAndDice::StaticClass());
        if (LoadedClass == nullptr) {
            TestTrue("SliceAndDice class should be loaded successfully", false);
            return;
        }

        TestSliceAndDice = BaseSpec.WorldHelper->GetWorld()->SpawnActor<ASliceAndDice>(LoadedClass);

        // FUNCTIONAL_CAMERA_TARGET(TestSliceAndDice)
    });

    AfterEach([this] {
        // Clear object references
        TestSliceAndDice = nullptr;

        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.TeardownBaseSpecEnvironment();
        // SPEC_BOILERPLATE_END
    });

    // Asset and Spawning
    Describe("Asset Validation", [this] {
        It("should spawn successfully from blueprint asset", [this] {
            // The test succeeds if BeforeEach completes without issues
            // TestSliceAndDice being valid means the asset loaded and spawned correctly
            TestTrue("SliceAndDice should be spawned", IsValid(TestSliceAndDice));
        });
    });

    // Common DefensiveStructure Tests
    Describe("Entity Properties", [this] {
        It("should have correct entity properties", [this] {
            // Test defensive structure specific properties
            DefensiveStructureTests_CommonTests::TestEntityProperties(this, TestSliceAndDice);
        });
    });

    Describe("Targeting Behavior", [this] {
        It("should handle its targetability correctly", [this] {
            DefensiveStructureTests_CommonTests::TestTargetability(this, TestSliceAndDice);
        });
    });

    // Blade Rotation and State Management
    Describe("Blade Rotation System", [this] {
        It("should activate blade hitbox when enemies are detected", [this] {
            // Arrange - place enemy in detection range
            AMockEnemy* MockEnemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestSliceAndDice, TestSliceAndDice->GetEnemyDetectionComponent()->GetDetectionRadius());

            // Act & Assert - blade should activate
            TestTrue("Blade should activate when enemy detected", 
                    FTestUtils::WaitForCondition(BaseSpec.WorldHelper.Get(), [this]() -> bool {
                        return TestSliceAndDice && IsValid(TestSliceAndDice) && 
                               TestSliceAndDice->GetHitboxState() == ESliceAndDiceHitboxState::Enabled;
                    }, 10.0f, TEXT("blade activation")));
            TestEqual("Hitbox state should be enabled", 
                     TestSliceAndDice->GetHitboxState(), ESliceAndDiceHitboxState::Enabled);
        });

        It("should deactivate blade hitbox when no enemies are present", [this] {
            // Arrange - place enemy, let blade activate, then remove enemy
            AMockEnemy* MockEnemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestSliceAndDice, TestSliceAndDice->GetEnemyDetectionComponent()->GetDetectionRadius());
            FTestUtils::WaitForCondition(BaseSpec.WorldHelper.Get(), [this]() -> bool {
                return TestSliceAndDice && IsValid(TestSliceAndDice) && 
                       TestSliceAndDice->GetHitboxState() == ESliceAndDiceHitboxState::Enabled;
            }, 5.0f, TEXT("blade activation"));
            TestEqual("Blade should initially be active", 
                     TestSliceAndDice->GetHitboxState(), ESliceAndDiceHitboxState::Enabled);

            // Act - destroy enemy and tick
            MockEnemy->Destroy();
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 60); // Give time for blade to slow down

            // Assert - blade should deactivate when no enemies
            TestEqual("Blade should deactivate when no enemies", 
                     TestSliceAndDice->GetHitboxState(), ESliceAndDiceHitboxState::Disabled);
        });

        It("should increase blade rotation speed when enemies are present", [this] {
            // Arrange - record initial rotation speed
            const float InitialRotationVelocity = TestSliceAndDice->GetCurrentBladeRotationVelocity();
            
            // Act - place enemy in detection range
            AMockEnemy* MockEnemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestSliceAndDice, TestSliceAndDice->GetEnemyDetectionComponent()->GetDetectionRadius());
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 30); // Allow acceleration
            
            // Assert - rotation speed should increase
            TestTrue("Blade rotation speed should increase when enemies detected", 
                    TestSliceAndDice->GetCurrentBladeRotationVelocity() > InitialRotationVelocity);
        });
    });

    // Combat and Damage
    Describe("Combat System", [this] {
        It("should damage enemies when blade is active", [this] {
            // Arrange - place enemy within blade reach
            AMockEnemy* MockEnemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestSliceAndDice, TestSliceAndDice->GetBladeRadius());

            MockEnemy->SetTestFlag(EMockEnemyFlags::Immortal);
            const float InitialHealth = MockEnemy->GetCurrentHealth();

            // Act & Assert
            TestTrue("SliceAndDice should damage enemy with blade",
                     FTestUtils::WaitForCondition(BaseSpec.WorldHelper.Get(), [MockEnemy]() -> bool {
                         return MockEnemy && IsValid(MockEnemy) && MockEnemy->WasDamagedByClass(ASliceAndDice::StaticClass());
                     }, 15.0f, TEXT("slice and dice damage")));
            TestTrue("Damage amount should be greater than zero", 
                    MockEnemy->LastDamagePayload.DamageAmount > 0.0f);
            TestTrue("Enemy health should be reduced", 
                    MockEnemy->GetCurrentHealth() < InitialHealth);
        });

        It("should respect hit delay between damage applications", [this] {
            // Arrange - place enemy within blade reach
            AMockEnemy* MockEnemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestSliceAndDice, TestSliceAndDice->GetBladeRadius());
            
            MockEnemy->SetTestFlag(EMockEnemyFlags::Immortal);
            
            // Wait for first hit
            FTestUtils::WaitForCondition(BaseSpec.WorldHelper.Get(), [MockEnemy]() -> bool {
                return MockEnemy && IsValid(MockEnemy) && MockEnemy->WasDamagedByClass(ASliceAndDice::StaticClass());
            }, 15.0f, TEXT("slice and dice damage"));
            const float HealthAfterFirstHit = MockEnemy->GetCurrentHealth();
            MockEnemy->ClearState(EMockEnemyState::HasBeenDamaged); // Reset to test for second hit
            
            // Act - tick for a short time (less than min hit delay)
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 2); // Very short time
            
            // Assert - should not have taken additional damage immediately
            TestTrue("Enemy should not take immediate additional damage due to hit delay", 
                    FMath::IsNearlyEqual(MockEnemy->GetCurrentHealth(), HealthAfterFirstHit, 0.1f));
        });
    });

    // State Management
    Describe("State Management", [this] {
        It("should not activate blade while in preview mode", [this] {
            // Arrange - put slice and dice in preview mode
            TestSliceAndDice->OnStartedPreviewing();
            AMockEnemy* Enemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestSliceAndDice, TestSliceAndDice->GetEnemyDetectionComponent()->GetDetectionRadius());
            
            // Act - tick to allow potential activation
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 60);
            
            // Assert - blade should remain inactive while previewing
            TestEqual("Blade should remain inactive while previewing", 
                     TestSliceAndDice->GetHitboxState(), ESliceAndDiceHitboxState::Disabled);
        });

        It("should start blade activation after being placed", [this] {
            // Arrange - start in preview, then place
            TestSliceAndDice->OnStartedPreviewing();
            AMockEnemy* Enemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestSliceAndDice, TestSliceAndDice->GetEnemyDetectionComponent()->GetDetectionRadius());
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 30);
            TestEqual("Blade should remain inactive while previewing", 
                     TestSliceAndDice->GetHitboxState(), ESliceAndDiceHitboxState::Disabled);
            
            // Act - place the slice and dice
            TestSliceAndDice->OnPlaced();
            
            // Assert - blade should now activate
            TestTrue("Blade should activate after placement", 
                    FTestUtils::WaitForCondition(BaseSpec.WorldHelper.Get(), [this]() -> bool {
                        return TestSliceAndDice && IsValid(TestSliceAndDice) && 
                               TestSliceAndDice->GetHitboxState() == ESliceAndDiceHitboxState::Enabled;
                    }, 10.0f, TEXT("blade activation")));
        });
    });

}