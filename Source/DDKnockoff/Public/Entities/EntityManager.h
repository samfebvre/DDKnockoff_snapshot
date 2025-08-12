#pragma once

#include "CoreMinimal.h"
#include "Core/ManagerBase.h"
#include "UObject/Object.h"
#include "Entities/Entity.h"
#include "EntityManager.generated.h"

class IEntity;

/**
 * Wrapper struct for array of entities to use in TMap collections.
 */
USTRUCT()
struct FEntityArray {
    GENERATED_BODY()

    UPROPERTY()
    TArray<TScriptInterface<IEntity>> Entities;
};

/**
 * Composite key structure for entity lookup by faction and type combination.
 */
USTRUCT()
struct FFactionTypeKey {
    GENERATED_BODY()

    FFactionTypeKey()
        : Faction(EFaction::None), Type(EEntityType::None) {}

    FFactionTypeKey(EFaction InFaction, EEntityType InType)
        : Faction(InFaction), Type(InType) {}

    UPROPERTY()
    EFaction Faction;

    UPROPERTY()
    EEntityType Type;

    bool operator==(const FFactionTypeKey& Other) const {
        return Faction == Other.Faction && Type == Other.Type;
    }

    friend uint32 GetTypeHash(const FFactionTypeKey& Key) {
        return HashCombine(GetTypeHash(static_cast<uint8>(Key.Faction)),
                           GetTypeHash(static_cast<uint8>(Key.Type)));
    }
};

/**
 * Central manager for tracking and organizing all entities in the game world.
 * Provides efficient lookup by faction, type, and composite keys for AI and gameplay systems.
 */
UCLASS()
class DDKNOCKOFF_API UEntityManager : public UManagerBase {
    GENERATED_BODY()

public:
    UEntityManager();

    // ManagerBase Interface
    virtual void Initialize() override;
    virtual void Deinitialize() override;

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEntityRemovedSignature,
                                                const TScriptInterface<IEntity>&,
                                                Entity);

    UPROPERTY(BlueprintAssignable, Category = "Entity Management")
    FOnEntityRemovedSignature OnEntityRemoved;

    // Entity registration

    /**
     * Register an entity with the manager for tracking and lookup.
     * @param Entity - Entity to register
     */
    virtual void RegisterEntity(IEntity* Entity);

    /**
     * Unregister an entity from the manager and fire removal event.
     * @param Entity - Entity to unregister
     */
    virtual void UnregisterEntity(IEntity* Entity);

    // Entity queries

    /**
     * Get all registered entities in the world.
     * @return Array of all entities
     */
    virtual const TArray<TScriptInterface<IEntity>>& GetAllEntities() const { return AllEntities; }

    /**
     * Find a specific entity by its unique identifier.
     * @param ID - Entity's unique ID
     * @return Entity interface or null if not found
     */
    virtual TScriptInterface<IEntity> GetEntityByID(const FGuid& ID) const;

    /**
     * Get all entities belonging to a specific faction.
     * @param Faction - Target faction
     * @return Array of entities in the faction
     */
    virtual TArray<TScriptInterface<IEntity>> GetEntitiesByFaction(EFaction Faction) const;

    /**
     * Get all entities of a specific type.
     * @param Type - Target entity type
     * @return Array of entities of the type
     */
    virtual TArray<TScriptInterface<IEntity>> GetEntitiesByType(EEntityType Type) const;

    /**
     * Get entities matching both faction and type criteria.
     * @param Faction - Target faction
     * @param Type - Target entity type
     * @return Array of matching entities
     */
    virtual TArray<TScriptInterface<IEntity>> GetEntitiesByFactionAndType(
        EFaction Faction,
        EEntityType Type) const;

private:
    // Entity storage and lookup tables

    UPROPERTY(Transient)
    TMap<FGuid, TScriptInterface<IEntity>> EntityMap;

    UPROPERTY(Transient)
    TMap<EFaction, FEntityArray> FactionMap;

    UPROPERTY(Transient)
    TMap<EEntityType, FEntityArray> TypeMap;

    UPROPERTY(Transient)
    TMap<FFactionTypeKey, FEntityArray> FactionAndTypeMap;

    UPROPERTY(Transient)
    TArray<TScriptInterface<IEntity>> AllEntities;
};
