#include "CoreMinimal.h"
#include "Debug/DebugInformationManager.h"
#include "Tests/Common/BaseSpec.h"
#include "Structures/Ballista/Ballista.h"
#include "Structures/Ballista/BallistaAmmo.h"
#include "Structures/Components/EnemyDetectionComponent.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/DefenseStructure/DefenseStructureTestHelpers.h"
#include "Tests/DefenseStructure/DefensiveStructure_CommonTests.h"
#include "Entities/EntityManager.h"
#include "Structures/StructurePlacementManager.h"

BEGIN_DEFINE_SPEC(FBallistaSpec,
                  "DDKnockoff.Structures.Ballista",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |
                  EAutomationTestFlags::ProductFilter)

    // SPEC_BOILERPLATE_BEGIN
    BaseSpec BaseSpec;
    // SPEC_BOILERPLATE_END
    
    TObjectPtr<ABallista> TestBallista;

END_DEFINE_SPEC(FBallistaSpec)

void FBallistaSpec::Define() {
    BeforeEach([this] {
        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.SetupBaseSpecEnvironment({UDebugInformationManager::StaticClass(),
                                           UEntityManager::StaticClass(),
                                           UStructurePlacementManager::StaticClass()});
        // SPEC_BOILERPLATE_END


        UClass* LoadedClass = FDefenseStructureTestHelpers::LoadDefenseStructureBlueprintClass(ABallista::StaticClass());
        if (LoadedClass == nullptr) {
            TestTrue("Ballista class should be loaded successfully", false);
            return;
        }

        TestBallista = BaseSpec.WorldHelper->GetWorld()->SpawnActor<ABallista>(LoadedClass);

        // FUNCTIONAL_CAMERA_TARGET(TestBallista)
    });

    AfterEach([this] {
        // Clear object references
        TestBallista = nullptr;

        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.TeardownBaseSpecEnvironment();
        // SPEC_BOILERPLATE_END
    });

    // Asset and Spawning
    Describe("Asset Validation", [this] {
        It("should spawn successfully from blueprint asset", [this] {
            // The test succeeds if BeforeEach completes without issues
            // TestBallista being valid means the asset loaded and spawned correctly
            TestTrue("Ballista should be spawned", IsValid(TestBallista));
        });
    });

    // Common DefensiveStructure Tests
    Describe("Entity Properties", [this] {
        It("should have correct entity properties", [this] {
            // Test defensive structure specific properties
            DefensiveStructureTests_CommonTests::TestEntityProperties(this, TestBallista);
        });
    });

    Describe("Targeting Behavior", [this] {
        It("should handle its targetability correctly", [this] {
            DefensiveStructureTests_CommonTests::TestTargetability(this, TestBallista);
        });
    });

    // Core Targeting and Detection
    Describe("Targeting System", [this] {
        It("should detect and target enemies within range", [this] {
            // Arrange - place enemy in detection range
            AMockEnemy* MockEnemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestBallista, TestBallista->GetEnemyDetectionComponent()->GetDetectionRadius());

            // Act - tick to allow detection and engagement
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);

            // Assert - should detect and target the specific enemy
            TestNotNull("Should detect and target enemy", TestBallista->GetTargetActor());
            TestEqual("Should target the spawned enemy", TestBallista->GetTargetActor(), static_cast<AActor*>(MockEnemy));
        });

        It("should target the closest enemy when multiple enemies are in range", [this] {
            // Arrange - create enemies at different distances
            const FVector BallistaPos = TestBallista->GetActorLocation();
            const float DetectionRange = TestBallista->GetEnemyDetectionComponent()->GetDetectionRadius();
            
            // Place far enemy first (should be targeted initially)
            const AMockEnemy* EnemyCDO = GetDefault<AMockEnemy>();
            const float EnemyHalfHeight = EnemyCDO->GetHalfHeight();
            const FVector FarEnemyLocation = BallistaPos + FVector(DetectionRange - 50.0f, 0.0f, EnemyHalfHeight);
            const FTransform FarEnemyTransform = FTransform(FRotator::ZeroRotator, FarEnemyLocation, FVector::OneVector);
            AMockEnemy* FarEnemy = BaseSpec.WorldHelper->GetWorld()->SpawnActor<AMockEnemy>(AMockEnemy::StaticClass(), FarEnemyTransform);
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);
            TestEqual("Should initially target far enemy", TestBallista->GetTargetActor(), static_cast<AActor*>(FarEnemy));
            
            // Act - spawn closer enemy
            const FVector CloseEnemyLocation = BallistaPos + FVector(DetectionRange - 100.0f, 0.0f, EnemyHalfHeight);
            const FTransform CloseEnemyTransform = FTransform(FRotator::ZeroRotator, CloseEnemyLocation, FVector::OneVector);
            AMockEnemy* CloseEnemy = BaseSpec.WorldHelper->GetWorld()->SpawnActor<AMockEnemy>(AMockEnemy::StaticClass(), CloseEnemyTransform);
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);
            
            // Assert - should switch to closer enemy
            TestEqual("Should switch to closer enemy", TestBallista->GetTargetActor(), static_cast<AActor*>(CloseEnemy));
        });

        It("should not target same faction enemies", [this] {
            // Arrange - create close friendly and far enemy
            const FVector BallistaPos = TestBallista->GetActorLocation();
            const float DetectionRange = TestBallista->GetEnemyDetectionComponent()->GetDetectionRadius();
            
            // Create a close "friendly" (Player faction) enemy
            const AMockEnemy* EnemyCDO = GetDefault<AMockEnemy>();
            const float EnemyHalfHeight = EnemyCDO->GetHalfHeight();
            const FVector CloseAllyLocation = BallistaPos + FVector(DetectionRange - 100.0f, 0.0f, EnemyHalfHeight);
            const FTransform CloseAllyTransform = FTransform(FRotator::ZeroRotator, CloseAllyLocation, FVector::OneVector);
            AMockEnemy* CloseAlly = BaseSpec.WorldHelper->GetWorld()->SpawnActorDeferred<AMockEnemy>(AMockEnemy::StaticClass(), CloseAllyTransform);
            CloseAlly->SetFaction(EFaction::Player); // Same faction as ballista
            CloseAlly->FinishSpawning(CloseAllyTransform);
            
            // Create a far actual enemy
            const FVector FarEnemyLocation = BallistaPos + FVector(DetectionRange - 50.0f, 0.0f, EnemyHalfHeight);
            const FTransform FarEnemyTransform = FTransform(FRotator::ZeroRotator, FarEnemyLocation, FVector::OneVector);
            AMockEnemy* FarEnemy = BaseSpec.WorldHelper->GetWorld()->SpawnActorDeferred<AMockEnemy>(AMockEnemy::StaticClass(), FarEnemyTransform);
            FarEnemy->SetFaction(EFaction::Enemy); // Different faction
            FarEnemy->FinishSpawning(FarEnemyTransform);
            
            // Act - tick to allow targeting
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);
            
            // Assert - should target the far enemy, not the close ally
            TestEqual("Should target enemy faction, not ally", TestBallista->GetTargetActor(), static_cast<AActor*>(FarEnemy));
        });

        It("should lose target when enemies are destroyed", [this] {
            // Arrange - place enemy, let ballista detect it, then remove
            AMockEnemy* MockEnemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestBallista, TestBallista->GetEnemyDetectionComponent()->GetDetectionRadius());
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);
            
            TestNotNull("Should initially detect enemy", TestBallista->GetTargetActor());
            TestEqual("Should initially target the spawned enemy", TestBallista->GetTargetActor(), static_cast<AActor*>(MockEnemy));
        
            // Act - destroy enemy and tick
            MockEnemy->Destroy();
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);
        
            // Assert - should no longer have a target
            TestNull("Should lose target when enemies removed", TestBallista->GetTargetActor());
        });

        It("should lose target when enemies move out of range", [this] {
            // Arrange - place enemy, let ballista detect it, then remove
            AMockEnemy* MockEnemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestBallista, TestBallista->GetEnemyDetectionComponent()->GetDetectionRadius());
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);

            TestNotNull("Should initially detect enemy", TestBallista->GetTargetActor());
            TestEqual("Should initially target the spawned enemy", TestBallista->GetTargetActor(), static_cast<AActor*>(MockEnemy));

            // Act - move enemy away and tick
            const float DetectionRange = TestBallista->GetEnemyDetectionComponent()->GetDetectionRadius();
            const float EnemyRadius = MockEnemy->GetRadius();
            MockEnemy->SetActorLocation(TestBallista->GetActorLocation() + FVector(DetectionRange + EnemyRadius + 500.0f, 0.0f, MockEnemy->GetHalfHeight()));
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);

            // Assert - should no longer have a target
            TestNull("Should lose target when enemies are moved", TestBallista->GetTargetActor());
        });
    });

    // Combat and Damage
    Describe("Combat System", [this] {
        It("should attack and damage enemies with ballista ammo", [this] {
            // Arrange
            AMockEnemy* MockEnemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestBallista, TestBallista->GetEnemyDetectionComponent()->GetDetectionRadius());
            MockEnemy->SetTestFlag(EMockEnemyFlags::Immortal);
            const float InitialHealth = MockEnemy->GetCurrentHealth();

            // Act & Assert
            TestTrue("Ballista should damage enemy with BallistaAmmo", 
                    FTestUtils::WaitForCondition(BaseSpec.WorldHelper.Get(), [MockEnemy, InitialHealth]() -> bool {
                        if (MockEnemy && IsValid(MockEnemy) && MockEnemy->GetCurrentHealth() < InitialHealth) {
                            return MockEnemy->WasDamagedByClass(ABallistaAmmo::StaticClass());
                        }
                        return false;
                    }, 10.0f, TEXT("ballista damage")));
            TestTrue("Damage amount should be greater than zero", 
                    MockEnemy->LastDamagePayload.DamageAmount > 0.0f);
            TestTrue("Enemy health should be reduced", 
                    MockEnemy->GetCurrentHealth() < InitialHealth);
        });
    });

    // State Management
    Describe("State Management", [this] {
        It("should not target enemies while in preview mode", [this] {
            // Arrange - put ballista in preview mode
            TestBallista->OnStartedPreviewing();
            AMockEnemy* Enemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestBallista, TestBallista->GetEnemyDetectionComponent()->GetDetectionRadius());
            
            // Act - tick to allow potential detection
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);
            
            // Assert - should not target while previewing
            TestNull("Should not target while previewing", TestBallista->GetTargetActor());
        });

        It("should start targeting enemies after being placed", [this] {
            // Arrange - start in preview, then place
            TestBallista->OnStartedPreviewing();
            AMockEnemy* Enemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestBallista, TestBallista->GetEnemyDetectionComponent()->GetDetectionRadius());
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);
            TestNull("Should not target while previewing", TestBallista->GetTargetActor());
            
            // Act - place the ballista
            TestBallista->OnPlaced();
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);
            
            // Assert - should now target the enemy
            TestEqual("Should target after placement", TestBallista->GetTargetActor(), static_cast<AActor*>(Enemy));
        });
    });
}