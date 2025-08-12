#include "CoreMinimal.h"
#include "Entities/EntityData.h"
#include "Tests/Common/BaseSpec.h"
#include "Entities/EntityManager.h"
#include "Mocks/MockEnemy.h"

BEGIN_DEFINE_SPEC(FEntityManagerSpec,
                  "DDKnockoff.Entities.EntityManager",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |
                  EAutomationTestFlags::ProductFilter)

    // SPEC_BOILERPLATE_BEGIN
    BaseSpec BaseSpec;
    // SPEC_BOILERPLATE_END
    
    TObjectPtr<UEntityManager> EntityManager;
    TObjectPtr<AMockEnemy> TestEntity;

END_DEFINE_SPEC(FEntityManagerSpec)

void FEntityManagerSpec::Define() {
    BeforeEach([this] {
        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.SetupBaseSpecEnvironment({UEntityManager::StaticClass()});
        // SPEC_BOILERPLATE_END

        UManagerHandlerSubsystem* ManagerHandler = BaseSpec.WorldHelper->GetWorld()->GetSubsystem<UManagerHandlerSubsystem>();
        EntityManager =ManagerHandler->GetManager<UEntityManager>();
        TestTrue("EntityManager should be available", EntityManager != nullptr);

        auto SpawnTransform = FTransform::Identity;
        TestEntity = BaseSpec.WorldHelper->GetWorld()->SpawnActorDeferred<AMockEnemy>(AMockEnemy::StaticClass(), SpawnTransform);
        TestEntity->SetFaction(EFaction::Player);
        TestEntity->FinishSpawning(SpawnTransform);
    });

    AfterEach([this] {
        TestEntity = nullptr;
        EntityManager = nullptr;

        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.TeardownBaseSpecEnvironment();
        // SPEC_BOILERPLATE_END
    });

    Describe("Entity Registration", [this] {
        It("should register and track entities", [this] {
            // Arrange
             int32 InitialCount = EntityManager->GetAllEntities().Num();

            // Act
            EntityManager->RegisterEntity(TestEntity);

            // Assert
            TestEqual("Should track registered entity", 
                     EntityManager->GetAllEntities().Num(), InitialCount + 1);
        });

        It("should handle null entity registration", [this] {
            // Arrange
             int32 InitialCount = EntityManager->GetAllEntities().Num();

            // Act
            EntityManager->RegisterEntity(nullptr);

            // Assert
            TestEqual("Should ignore null entities", 
                     EntityManager->GetAllEntities().Num(), InitialCount);
        });
    });

    Describe("Entity Unregistration", [this] {
        It("should remove entities when unregistered", [this] {
            // Arrange
             int32 InitialCount = EntityManager->GetAllEntities().Num();

            // Act
            EntityManager->UnregisterEntity(TestEntity);

            // Assert
            TestEqual("Should remove entity", EntityManager->GetAllEntities().Num(), InitialCount - 1);
        });

        It("should handle unregistering null entities", [this] {
            // Arrange
             int32 InitialCount = EntityManager->GetAllEntities().Num();

            // Act
            EntityManager->UnregisterEntity(nullptr);

            // Assert
            TestEqual("Should ignore null entities", EntityManager->GetAllEntities().Num(), InitialCount);
        });
    });

    Describe("Entity Queries", [this] {
        It("should find entities by faction", [this] {
            // Act
            TArray<TScriptInterface<IEntity>> PlayerEntities = EntityManager->GetEntitiesByFaction(EFaction::Player);
            TArray<TScriptInterface<IEntity>> EnemyEntities = EntityManager->GetEntitiesByFaction(EFaction::Enemy);

            // Assert
            TestEqual("Should find player entity", PlayerEntities.Num(), 1);
            TestEqual("Should not find enemy entities", EnemyEntities.Num(), 0);
            TestEqual("Found entity should be test entity", PlayerEntities[0]->GetActor(), static_cast<AActor*>(TestEntity));
        });

        It("should find entities by ID", [this] {
            // Arrange
             FGuid EntityID = TestEntity->GetEntityData()->GetID();

            // Act
            TScriptInterface<IEntity> FoundEntity = EntityManager->GetEntityByID(EntityID);

            // Assert
            TestNotNull("Should find entity by ID", FoundEntity.GetObject());
            TestEqual("Found entity should be test entity", FoundEntity->GetActor(), static_cast<AActor*>(TestEntity));
        });
    });
}