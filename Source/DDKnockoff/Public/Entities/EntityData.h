#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EntityData.generated.h"

/**
 * Data container for entity identification and persistent state.
 * Provides unique ID generation and management for all game entities.
 * Automatically instantiated for each entity that requires tracking.
 */
UCLASS(DefaultToInstanced, Blueprintable)
class DDKNOCKOFF_API UEntityData : public UObject {
    GENERATED_BODY()

public:
    /**
     * Get the unique identifier for this entity.
     * @return Entity's unique GUID
     */
    FGuid GetID() const { return ID; }

    /**
     * Initialize the entity with a new unique ID.
     * Should be called once during entity creation.
     */
    void InitialiseID();

private:
    UPROPERTY(Transient)
    FGuid ID;
};
