#include "LevelLogic/Spawner.h"

#include "Entities/Entity.h"
#include "../../Public/LevelLogic/DoorComponent.h"

// Sets default values
ASpawner::ASpawner() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

// Called every frame
void ASpawner::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void ASpawner::SpawnActor(const TSubclassOf<AActor>& EntityToSpawn) const {
    if (EntityToSpawn->ImplementsInterface(UEntity::StaticClass())) {
        const IEntity& EntityInterface = *Cast<IEntity>(EntityToSpawn->GetDefaultObject());

        const auto SpawnRot = GetActorRotation();
        const auto HalfHeight = EntityInterface.GetHalfHeight();
        // Set the spawn location to be above the ground by the capsule half height
        const auto spawnLoc = GetActorLocation() + FVector(0.0f, 0.0f, HalfHeight);

        AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(EntityToSpawn, spawnLoc, SpawnRot);
    } else { UE_LOG(LogTemp, Warning, TEXT("Entity class does not implement IEntity interface!")); }
}

void ASpawner::OpenDoors() const {
    for (const TObjectPtr<AActor>& Door : LinkedDoors) {
        if (Door != nullptr) {
            // Get the door component and call OpenDoor
            UDoorComponent* DoorComponent = Door->FindComponentByClass<UDoorComponent>();
            if (DoorComponent) { DoorComponent->OpenDoor(); } else {
                UE_LOG(LogTemp,
                       Warning,
                       TEXT("Spawner: Linked door does not have a UDoorComponent!"));
            }
        }
    }
}

void ASpawner::CloseDoors() const {
    for (const TObjectPtr<AActor>& Door : LinkedDoors) {
        if (Door != nullptr) {
            // Get the door component and call CloseDoor
            UDoorComponent* DoorComponent = Door->FindComponentByClass<UDoorComponent>();
            if (DoorComponent) { DoorComponent->CloseDoor(); } else {
                UE_LOG(LogTemp,
                       Warning,
                       TEXT("Spawner: Linked door does not have a UDoorComponent!"));
            }
        }
    }
}
