#include "CoreMinimal.h"
#include "Core/ManagerHandlerSubsystem.h"
#include "Debug/DebugInformationManager.h"
#include "Entities/EntityManager.h"
#include "Tests/Common/BaseSpec.h"
#include "Structures/BounceBlockade/BouncerBlockade.h"
#include "Structures/BounceBlockade/BouncerBlockadeAnimInstance.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/DefenseStructure/DefenseStructureTestHelpers.h"
#include "Tests/DefenseStructure/DefensiveStructure_CommonTests.h"
#include "Mocks/MockEnemy.h"
#include "Structures/StructurePlacementManager.h"
#include "Structures/Components/EnemyDetectionComponent.h"

BEGIN_DEFINE_SPEC(FBouncerBlockadeSpec,
                  "DDKnockoff.Structures.BouncerBlockade",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |
                  EAutomationTestFlags::ProductFilter)

    // SPEC_BOILERPLATE_BEGIN
    BaseSpec BaseSpec;
    // SPEC_BOILERPLATE_END
    
    TObjectPtr<ABouncerBlockade> TestBouncerBlockade;

END_DEFINE_SPEC(FBouncerBlockadeSpec)

void FBouncerBlockadeSpec::Define() {
    BeforeEach([this] {
        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.SetupBaseSpecEnvironment({UDebugInformationManager::StaticClass(),
                                           UEntityManager::StaticClass(),
                                           UStructurePlacementManager::StaticClass()});
        // SPEC_BOILERPLATE_END

        UClass* LoadedClass = FDefenseStructureTestHelpers::LoadDefenseStructureBlueprintClass(ABouncerBlockade::StaticClass());
        if (LoadedClass == nullptr) {
            TestTrue("BouncerBlockade class should be loaded successfully", false);
            return;
        }

        TestBouncerBlockade = BaseSpec.WorldHelper->GetWorld()->SpawnActor<ABouncerBlockade>(LoadedClass);

        // FUNCTIONAL_CAMERA_TARGET(TestBouncerBlockade)
    });

    AfterEach([this] {
        // Clear object references
        TestBouncerBlockade = nullptr;

        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.TeardownBaseSpecEnvironment();
        // SPEC_BOILERPLATE_END
    });

    // Asset and Spawning
    Describe("Asset Validation", [this] {
        It("should spawn successfully from blueprint asset", [this] {
            // The test succeeds if BeforeEach completes without issues
            // TestBouncerBlockade being valid means the asset loaded and spawned correctly
            TestTrue("BouncerBlockade should be spawned", IsValid(TestBouncerBlockade));
        });
    });

    // Common DefensiveStructure Tests
    Describe("Entity Properties", [this] {
        It("should have correct entity properties", [this] {
            // Test defensive structure specific properties
            DefensiveStructureTests_CommonTests::TestEntityProperties(this, TestBouncerBlockade);
        });
    });

    Describe("Targeting Behavior", [this] {
        It("should handle its targetability correctly", [this] {
            DefensiveStructureTests_CommonTests::TestTargetability(this, TestBouncerBlockade);
        });
    });

    // Core Detection and State Management
    Describe("Detection System", [this] {
        It("should detect enemies within range and switch to attacking state", [this] {
            // Arrange - place enemy in detection range
            AMockEnemy* MockEnemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestBouncerBlockade, TestBouncerBlockade->GetEnemyDetectionComponent()->GetDetectionRadius());

            // Act - tick to allow detection
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);

            // Assert - should switch to attacking state
            TestEqual("Should be in attacking state when enemy detected", 
                     TestBouncerBlockade->GetCurrentState(), EDefensiveStructureState::Attacking);
        });

        It("should return to idle state when no enemies in range", [this] {
            // Arrange - place enemy in range, then remove it
            AMockEnemy* MockEnemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestBouncerBlockade, TestBouncerBlockade->GetEnemyDetectionComponent()->GetDetectionRadius());
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);
            TestEqual("Should initially be attacking", 
                     TestBouncerBlockade->GetCurrentState(), EDefensiveStructureState::Attacking);

            // Act - destroy enemy and tick
            MockEnemy->Destroy();
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);

            // Assert - should return to idle state
            TestEqual("Should return to idle when no enemies", 
                     TestBouncerBlockade->GetCurrentState(), EDefensiveStructureState::Idle);
        });
    });

    // Animation and Attack System
    Describe("Animation System", [this] {
        It("should start attack animation when enemy detected", [this] {
            // Arrange
            AMockEnemy* MockEnemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestBouncerBlockade, TestBouncerBlockade->GetEnemyDetectionComponent()->GetDetectionRadius());

            // Act & Assert
            TestTrue("Should start attack animation when enemy detected", 
                    FTestUtils::WaitForCondition(BaseSpec.WorldHelper.Get(), [this]() -> bool {
                        return TestBouncerBlockade && IsValid(TestBouncerBlockade) && 
                               TestBouncerBlockade->GetAnimInstance() && 
                               TestBouncerBlockade->GetAnimInstance()->CurrentBouncerBlockadeAnimationState == EBouncerBlockadeAnimationState::Attacking;
                    }, 5.0f, TEXT("bouncer attack animation")));
        });

        It("should not attack while in preview mode", [this] {
            // Arrange - put bouncer in preview mode
            TestBouncerBlockade->OnStartedPreviewing();
            AMockEnemy* Enemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestBouncerBlockade, TestBouncerBlockade->GetEnemyDetectionComponent()->GetDetectionRadius());
            
            // Act - tick to allow potential detection
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 10);
            
            // Assert - should remain idle while previewing
            TestEqual("Should remain idle while previewing", 
                     TestBouncerBlockade->GetAnimInstance()->CurrentBouncerBlockadeAnimationState, 
                     EBouncerBlockadeAnimationState::Idle);
        });

        It("should start attacking after being placed", [this] {
            // Arrange - start in preview, then place
            TestBouncerBlockade->OnStartedPreviewing();
            AMockEnemy* Enemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestBouncerBlockade, TestBouncerBlockade->GetEnemyDetectionComponent()->GetDetectionRadius());
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 5);
            TestEqual("Should remain idle while previewing", 
                     TestBouncerBlockade->GetAnimInstance()->CurrentBouncerBlockadeAnimationState, 
                     EBouncerBlockadeAnimationState::Idle);
            
            // Act - place the bouncer
            TestBouncerBlockade->OnPlaced();
            
            // Assert - should now start attacking
            TestTrue("Should start attacking after placement", 
                    FTestUtils::WaitForCondition(BaseSpec.WorldHelper.Get(), [this]() -> bool {
                        return TestBouncerBlockade && IsValid(TestBouncerBlockade) && 
                               TestBouncerBlockade->GetAnimInstance() && 
                               TestBouncerBlockade->GetAnimInstance()->CurrentBouncerBlockadeAnimationState == EBouncerBlockadeAnimationState::Attacking;
                    }, 5.0f, TEXT("bouncer attack animation after placement")));
        });
    });

    // Combat and Damage
    Describe("Combat System", [this] {
        It("should attack and damage enemies with melee damage", [this] {
            // Arrange
            AMockEnemy* MockEnemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestBouncerBlockade, TestBouncerBlockade->GetEnemyDetectionComponent()->GetDetectionRadius());
            MockEnemy->SetTestFlag(EMockEnemyFlags::Immortal);
            const float InitialHealth = MockEnemy->GetCurrentHealth();

            // Act & Assert
            TestTrue("BouncerBlockade should damage enemy with melee damage", 
                    FTestUtils::WaitForCondition(BaseSpec.WorldHelper.Get(), [MockEnemy, InitialHealth]() -> bool {
                        return MockEnemy && IsValid(MockEnemy) && 
                               MockEnemy->GetCurrentHealth() < InitialHealth && 
                               MockEnemy->LastDamagePayload.DamageType == EDDDamageType::Melee;
                    }, 10.0f, TEXT("bouncer blockade melee damage")));
            TestTrue("Damage amount should be greater than zero", 
                    MockEnemy->LastDamagePayload.DamageAmount > 0.0f);
            TestTrue("Enemy health should be reduced", 
                    MockEnemy->GetCurrentHealth() < InitialHealth);
        });
    });
}