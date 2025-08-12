#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spawner.generated.h"

class UDoorComponent;
class UEntity;
class IEntity;

/**
 * Actor responsible for spawning entities at designated world locations.
 * Each spawner has a unique ID for wave management coordination.
 * Handles entity spawning with proper placement and validation.
 */
UCLASS()
class DDKNOCKOFF_API ASpawner : public AActor {
    GENERATED_BODY()

public:
    ASpawner();

    // Actor lifecycle
    virtual void Tick(float DeltaTime) override;

    // Spawning functionality

    /**
     * Spawn an actor instance at this spawner's location.
     * @param EntityToSpawn - Class of entity to spawn (must implement Entity interface)
     */
    void SpawnActor(const TSubclassOf<AActor>& EntityToSpawn) const;

    // Configuration

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    int SpawnerID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    TArray<TObjectPtr<AActor>> LinkedDoors;

    void OpenDoors() const;
    void CloseDoors() const;
};
