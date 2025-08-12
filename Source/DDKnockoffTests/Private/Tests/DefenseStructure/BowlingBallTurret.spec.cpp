#include "CoreMinimal.h"
#include "Core/ManagerHandlerSubsystem.h"
#include "Debug/DebugInformationManager.h"
#include "Tests/Common/BaseSpec.h"
#include "Structures/BowlingBallTurret/BowlingBallTurret.h"
#include "Structures/BowlingBallTurret/BowlingBallTurretAmmo.h"
#include "Structures/Components/EnemyDetectionComponent.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/DefenseStructure/DefenseStructureTestHelpers.h"
#include "Tests/DefenseStructure/DefensiveStructure_CommonTests.h"
#include "Entities/EntityManager.h"
#include "Mocks/MockEnemy.h"
#include "Structures/StructurePlacementManager.h"

BEGIN_DEFINE_SPEC(FBowlingBallTurretSpec,
                  "DDKnockoff.Structures.BowlingBallTurret",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |
                  EAutomationTestFlags::ProductFilter)

    // SPEC_BOILERPLATE_BEGIN
    BaseSpec BaseSpec;
    // SPEC_BOILERPLATE_END
    
    TObjectPtr<ABowlingBallTurret> TestBowlingBallTurret;

END_DEFINE_SPEC(FBowlingBallTurretSpec)

void FBowlingBallTurretSpec::Define() {
    BeforeEach([this] {
        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.SetupBaseSpecEnvironment({UDebugInformationManager::StaticClass(),
                                           UEntityManager::StaticClass(),
                                           UStructurePlacementManager::StaticClass()});
        // SPEC_BOILERPLATE_END

        UClass* LoadedClass = FDefenseStructureTestHelpers::LoadDefenseStructureBlueprintClass(ABowlingBallTurret::StaticClass());
        if (LoadedClass == nullptr) {
            TestTrue("BowlingBallTurret class should be loaded successfully", false);
            return;
        }

        TestBowlingBallTurret = BaseSpec.WorldHelper->GetWorld()->SpawnActor<ABowlingBallTurret>(LoadedClass);

        // FUNCTIONAL_CAMERA_TARGET(TestBowlingBallTurret)
    });

    AfterEach([this] {
        // Clear object references
        TestBowlingBallTurret = nullptr;

        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.TeardownBaseSpecEnvironment();
        // SPEC_BOILERPLATE_END
    });

    // Asset and Spawning
    Describe("Asset Validation", [this] {
        It("should spawn successfully from blueprint asset", [this] {
            // The test succeeds if BeforeEach completes without issues
            // TestBowlingBallTurret being valid means the asset loaded and spawned correctly
            TestTrue("BowlingBallTurret should be spawned", IsValid(TestBowlingBallTurret));
        });
    });

    // Common DefensiveStructure Tests
    Describe("Entity Properties", [this] {
        It("should have correct entity properties", [this] {
            // Test defensive structure specific properties
            DefensiveStructureTests_CommonTests::TestEntityProperties(this, TestBowlingBallTurret);
        });
    });

    Describe("Targeting Behavior", [this] {
        It("should handle its targetability correctly", [this] {
            DefensiveStructureTests_CommonTests::TestTargetability(this, TestBowlingBallTurret);
        });
    });

    // Core Targeting and Detection
    Describe("Targeting System", [this] {
        It("should detect and target enemies within range", [this] {
            // Arrange - place enemy in detection range
            AMockEnemy* MockEnemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestBowlingBallTurret, TestBowlingBallTurret->EnemyDetectionComponent->GetDetectionRadius());

            // Act - tick to allow detection and engagement
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);

            // Assert - should detect and target the specific enemy
            TestNotNull("Should detect and target enemy", TestBowlingBallTurret->TargetActor.Get());
            TestEqual("Should target the spawned enemy", TestBowlingBallTurret->TargetActor.Get(), static_cast<AActor*>(MockEnemy));
        });

        It("should target the closest enemy when multiple enemies are in range", [this] {
            // Arrange - create enemies at different distances
            const FVector TurretPos = TestBowlingBallTurret->GetActorLocation();
            const float DetectionRange = TestBowlingBallTurret->EnemyDetectionComponent->GetDetectionRadius();
            
            // Place far enemy first (should be targeted initially)
            const AMockEnemy* EnemyCDO = GetDefault<AMockEnemy>();
            const float EnemyHalfHeight = EnemyCDO->GetHalfHeight();
            const FVector FarEnemyLocation = TurretPos + FVector(DetectionRange - 50.0f, 0.0f, EnemyHalfHeight);
            const FTransform FarEnemyTransform = FTransform(FRotator::ZeroRotator, FarEnemyLocation, FVector::OneVector);
            AMockEnemy* FarEnemy = BaseSpec.WorldHelper->GetWorld()->SpawnActor<AMockEnemy>(AMockEnemy::StaticClass(), FarEnemyTransform);
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);
            TestEqual("Should initially target far enemy", TestBowlingBallTurret->TargetActor.Get(), static_cast<AActor*>(FarEnemy));
            
            // Act - spawn closer enemy
            const FVector CloseEnemyLocation = TurretPos + FVector(DetectionRange - 100.0f, 0.0f, EnemyHalfHeight);
            const FTransform CloseEnemyTransform = FTransform(FRotator::ZeroRotator, CloseEnemyLocation, FVector::OneVector);
            AMockEnemy* CloseEnemy = BaseSpec.WorldHelper->GetWorld()->SpawnActor<AMockEnemy>(AMockEnemy::StaticClass(), CloseEnemyTransform);
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);
            
            // Assert - should switch to closer enemy
            TestEqual("Should switch to closer enemy", TestBowlingBallTurret->TargetActor.Get(), static_cast<AActor*>(CloseEnemy));
        });

        It("should not target same faction enemies", [this] {
            // Arrange - create close friendly and far enemy
            const FVector TurretPos = TestBowlingBallTurret->GetActorLocation();
            const float DetectionRange = TestBowlingBallTurret->EnemyDetectionComponent->GetDetectionRadius();
            
            // Create a close "friendly" (Player faction) enemy
            const AMockEnemy* EnemyCDO = GetDefault<AMockEnemy>();
            const float EnemyHalfHeight = EnemyCDO->GetHalfHeight();
            const FVector CloseAllyLocation = TurretPos + FVector(DetectionRange - 100.0f, 0.0f, EnemyHalfHeight);
            const FTransform CloseAllyTransform = FTransform(FRotator::ZeroRotator, CloseAllyLocation, FVector::OneVector);
            AMockEnemy* CloseAlly = BaseSpec.WorldHelper->GetWorld()->SpawnActorDeferred<AMockEnemy>(AMockEnemy::StaticClass(), CloseAllyTransform);
            CloseAlly->SetFaction(EFaction::Player); // Same faction as turret
            CloseAlly->FinishSpawning(CloseAllyTransform);
            
            // Create a far actual enemy
            const FVector FarEnemyLocation = TurretPos + FVector(DetectionRange - 50.0f, 0.0f, EnemyHalfHeight);
            const FTransform FarEnemyTransform = FTransform(FRotator::ZeroRotator, FarEnemyLocation, FVector::OneVector);
            AMockEnemy* FarEnemy = BaseSpec.WorldHelper->GetWorld()->SpawnActorDeferred<AMockEnemy>(AMockEnemy::StaticClass(), FarEnemyTransform);
            FarEnemy->SetFaction(EFaction::Enemy); // Different faction
            FarEnemy->FinishSpawning(FarEnemyTransform);
            
            // Act - tick to allow targeting
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);
            
            // Assert - should target the far enemy, not the close ally
            TestEqual("Should target enemy faction, not ally", TestBowlingBallTurret->TargetActor.Get(), static_cast<AActor*>(FarEnemy));
        });

        It("should lose target when enemies are destroyed", [this] {
            // Arrange - place enemy, let turret detect it, then remove
            AMockEnemy* MockEnemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestBowlingBallTurret, TestBowlingBallTurret->EnemyDetectionComponent->GetDetectionRadius());
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);
            
            TestNotNull("Should initially detect enemy", TestBowlingBallTurret->TargetActor.Get());
            TestEqual("Should initially target the spawned enemy", TestBowlingBallTurret->TargetActor.Get(), static_cast<AActor*>(MockEnemy));
        
            // Act - destroy enemy and tick
            MockEnemy->Destroy();
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);
        
            // Assert - should no longer have a target
            TestNull("Should lose target when enemies removed", TestBowlingBallTurret->TargetActor.Get());
        });

        It("should lose target when enemies move out of range", [this] {
            // Arrange - place enemy, let turret detect it, then remove
            AMockEnemy* MockEnemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestBowlingBallTurret, TestBowlingBallTurret->EnemyDetectionComponent->GetDetectionRadius());
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);

            TestNotNull("Should initially detect enemy", TestBowlingBallTurret->TargetActor.Get());
            TestEqual("Should initially target the spawned enemy", TestBowlingBallTurret->TargetActor.Get(), static_cast<AActor*>(MockEnemy));

            // Act - move enemy away and tick
            const float DetectionRange = TestBowlingBallTurret->EnemyDetectionComponent->GetDetectionRadius();
            const float EnemyRadius = MockEnemy->GetRadius();
            MockEnemy->SetActorLocation(TestBowlingBallTurret->GetActorLocation() + FVector(DetectionRange + EnemyRadius + 500.0f, 0.0f, MockEnemy->GetHalfHeight()));
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);

            // Assert - should no longer have a target
            TestNull("Should lose target when enemies are moved", TestBowlingBallTurret->TargetActor.Get());
        });
    });

    // Combat and Damage
    Describe("Combat System", [this] {
        It("should attack and damage enemies with bowling ball ammo", [this] {
            // Arrange
            AMockEnemy* MockEnemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestBowlingBallTurret, TestBowlingBallTurret->EnemyDetectionComponent->GetDetectionRadius());
            MockEnemy->SetTestFlag(EMockEnemyFlags::Immortal);
            const float InitialHealth = MockEnemy->GetCurrentHealth();

            // Act & Assert
            TestTrue("BowlingBallTurret should damage enemy with BowlingBallTurretAmmo", 
                    FTestUtils::WaitForCondition(BaseSpec.WorldHelper.Get(), [MockEnemy, InitialHealth]() -> bool {
                        return MockEnemy && IsValid(MockEnemy) && 
                               MockEnemy->WasDamagedByClass(ABowlingBallTurretAmmo::StaticClass());
                    }, 10.0f, TEXT("bowling ball turret damage")));
            TestTrue("Damage amount should be greater than zero", 
                    MockEnemy->LastDamagePayload.DamageAmount > 0.0f);
            TestTrue("Enemy health should be reduced", 
                    MockEnemy->GetCurrentHealth() < InitialHealth);
        });
    });

    // State Management
    Describe("State Management", [this] {
        It("should not target enemies while in preview mode", [this] {
            // Arrange - put turret in preview mode
            TestBowlingBallTurret->OnStartedPreviewing();
            AMockEnemy* Enemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestBowlingBallTurret, TestBowlingBallTurret->EnemyDetectionComponent->GetDetectionRadius());
            
            // Act - tick to allow potential detection
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);
            
            // Assert - should not target while previewing
            TestNull("Should not target while previewing", TestBowlingBallTurret->TargetActor.Get());
        });

        It("should start targeting enemies after being placed", [this] {
            // Arrange - start in preview, then place
            TestBowlingBallTurret->OnStartedPreviewing();
            AMockEnemy* Enemy = FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(BaseSpec.WorldHelper->GetWorld(), TestBowlingBallTurret, TestBowlingBallTurret->EnemyDetectionComponent->GetDetectionRadius());
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);
            TestNull("Should not target while previewing", TestBowlingBallTurret->TargetActor.Get());
            
            // Act - place the turret
            TestBowlingBallTurret->OnPlaced();
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);
            
            // Assert - should now target the enemy
            TestEqual("Should target after placement", TestBowlingBallTurret->TargetActor.Get(), static_cast<AActor*>(Enemy));
        });
    });
}