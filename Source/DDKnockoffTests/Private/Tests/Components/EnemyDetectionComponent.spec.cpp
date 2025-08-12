#include "CoreMinimal.h"
#include "Tests/Common/BaseSpec.h"
#include "Structures/Components/EnemyDetectionComponent.h"
#include "Mocks/MockEnemy.h"
#include "Components/SphereComponent.h"
#include "Entities/EntityManager.h"

BEGIN_DEFINE_SPEC(FEnemyDetectionComponentSpec,
                  "DDKnockoff.Components.EnemyDetectionComponent",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |
                  EAutomationTestFlags::ProductFilter)

    // SPEC_BOILERPLATE_BEGIN
    BaseSpec BaseSpec;
    // SPEC_BOILERPLATE_END
    
    TObjectPtr<AMockEnemy> OwnerActor;
    TObjectPtr<UEnemyDetectionComponent> DetectionComponent;

END_DEFINE_SPEC(FEnemyDetectionComponentSpec)

void FEnemyDetectionComponentSpec::Define() {
    BeforeEach([this] {
        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.SetupBaseSpecEnvironment({UEntityManager::StaticClass() });
        // SPEC_BOILERPLATE_END

        // Create owner actor (Player faction)
        auto SpawnTransform = FTransform::Identity;
        OwnerActor = BaseSpec.WorldHelper->GetWorld()->SpawnActorDeferred<AMockEnemy>(AMockEnemy::StaticClass(), SpawnTransform);
        OwnerActor->SetFaction(EFaction::Player);
        OwnerActor->FinishSpawning(SpawnTransform);

        // Create and setup detection component
        DetectionComponent = NewObject<UEnemyDetectionComponent>(OwnerActor);
        DetectionComponent->SetDetectionRadiusForTesting(500.0f);
        OwnerActor->AddInstanceComponent(DetectionComponent);
        
        // Trigger component registration and initialization
        // DetectionComponent->OnRegister();
        // DetectionComponent->BeginPlay();
    });

    AfterEach([this] {
        // Clear object references
        DetectionComponent = nullptr;
        OwnerActor = nullptr;

        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.TeardownBaseSpecEnvironment();
        // SPEC_BOILERPLATE_END
    });

    Describe("Faction-Based Detection", [this] {
        It("should detect enemies of different faction", [this] {
            // Arrange - create enemy actor close to owner
                auto SpawnLocation = OwnerActor->GetActorLocation() + FVector(100.0f, 0.0f, 0.0f);
            auto SpawnTransform = FTransform(FRotator::ZeroRotator, SpawnLocation);
            AMockEnemy* EnemyActor = BaseSpec.WorldHelper->GetWorld()->SpawnActor<AMockEnemy>( AMockEnemy::StaticClass(), SpawnTransform);
            
            // Simulate the enemy being in detection range
            TArray<AActor*> OverlappingActors = {EnemyActor};
            DetectionComponent->GetDetectionSphere()->SetGenerateOverlapEvents(true);
            
            // Manually set overlapping actors (simulating sphere overlap)
            // We need to use the sphere's overlap detection
            DetectionComponent->GetDetectionSphere()->GetOverlappingActors(OverlappingActors);
            
            // Act - tick to allow detection
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);
            FEnemyDetectionResult EnemyDetectionResult = DetectionComponent->DetectEnemies();
            
            // Assert
            TestTrue("Should detect enemy of different faction", EnemyDetectionResult.bEnemiesInRange);
            TestEqual("Should identify the enemy as closest", EnemyDetectionResult.ClosestEnemy, TObjectPtr<AActor>(EnemyActor));
        });

        It("should not detect allies of same faction", [this] {
            // Arrange - create ally actor close to owner
                auto spawnTransform = FTransform(FRotator::ZeroRotator, OwnerActor->GetActorLocation() + FVector(100.0f, 0.0f, 0.0f));
            AMockEnemy* AllyActor = BaseSpec.WorldHelper->GetWorld()->SpawnActorDeferred<AMockEnemy>( AMockEnemy::StaticClass(), spawnTransform);
            AllyActor->SetFaction(EFaction::Player); // Same faction as owner
            AllyActor->FinishSpawning(spawnTransform);

            // Act - tick to allow detection
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 1);
            FEnemyDetectionResult EnemyDetectionResult = DetectionComponent->DetectEnemies();
            
            // Assert
            TestFalse("Should not detect ally of same faction", EnemyDetectionResult.bEnemiesInRange);
            TestNull("Closest enemy should be null when no enemies detected", EnemyDetectionResult.ClosestEnemy);
        });
    });

    Describe("Distance-Based Selection", [this] {
        It("should select closest enemy when multiple enemies are in range", [this] {
            // Arrange - create two enemies at different distances
                auto SpawnTransform = FTransform::Identity + FTransform(FRotator::ZeroRotator, OwnerActor->GetActorLocation() + FVector(400.0f, 0.0f, 0.0f));
            AMockEnemy* FarEnemy = BaseSpec.WorldHelper->GetWorld()->SpawnActorDeferred<AMockEnemy>( AMockEnemy::StaticClass(), SpawnTransform);
            FarEnemy->SetFaction(EFaction::Enemy);
            FarEnemy->FinishSpawning(SpawnTransform);

            SpawnTransform = FTransform::Identity + FTransform(FRotator::ZeroRotator, OwnerActor->GetActorLocation() + FVector(200.0f, 0.0f, 0.0f));
            AMockEnemy* CloseEnemy = BaseSpec.WorldHelper->GetWorld()->SpawnActor<AMockEnemy>( AMockEnemy::StaticClass(), SpawnTransform);
            
            // Simulate both enemies being in detection sphere
            TArray<AActor*> OverlappingActors = {FarEnemy, CloseEnemy};

            // Act - tick to allow detection
            FTestUtils::TickMultipleFrames(BaseSpec.WorldHelper.Get(), 3); 
            FEnemyDetectionResult EnemyDetectionResult = DetectionComponent->DetectEnemies();
            
            // Assert
            TestTrue("Should detect enemies when multiple are in range", EnemyDetectionResult.bEnemiesInRange);
            TestEqual("Should select closest enemy", EnemyDetectionResult.ClosestEnemy, TObjectPtr<AActor>(CloseEnemy));
        });
    });
}