#include "Entities/EntityManager.h"
#include "Entities/Entity.h"
#include "Entities/EntityData.h"

UEntityManager::UEntityManager() {
    // No initialization needed
}

void UEntityManager::Deinitialize() {}

void UEntityManager::Initialize() {}

void UEntityManager::RegisterEntity(IEntity* Entity) {
    if (!Entity) { return; }

    UObject* Object = Cast<UObject>(Entity);
    if (!Object || !Object->GetClass()->ImplementsInterface(UEntity::StaticClass())) { return; }

    UEntityData* EntityData = Entity->GetEntityData();
    if (!EntityData) { return; }

    // Create interface reference
    TScriptInterface<IEntity> InterfaceRef;
    InterfaceRef.SetObject(Object);
    InterfaceRef.SetInterface(Entity);

    // Add to main map
    EntityData->InitialiseID();
    EntityMap.Add(EntityData->GetID(), InterfaceRef);

    // Add to faction map
    const EFaction Faction = Entity->GetFaction();
    if (!FactionMap.Contains(Faction)) { FactionMap.Add(Faction, FEntityArray()); }
    FactionMap[Faction].Entities.Add(InterfaceRef);

    // Add to type map
    const EEntityType Type = Entity->GetEntityType();
    if (!TypeMap.Contains(Type)) { TypeMap.Add(Type, FEntityArray()); }
    TypeMap[Type].Entities.Add(InterfaceRef);

    // Add to faction and type map
    const FFactionTypeKey FactionAndType(Faction, Type);
    if (!FactionAndTypeMap.Contains(FactionAndType)) {
        FactionAndTypeMap.Add(FactionAndType, FEntityArray());
    }
    FactionAndTypeMap[FactionAndType].Entities.Add(InterfaceRef);

    // Update cached array
    AllEntities.Add(InterfaceRef);
}

void UEntityManager::UnregisterEntity(IEntity* Entity) {
    if (!Entity) { return; }

    const UEntityData* EntityData = Entity->GetEntityData();
    if (!EntityData) { return; }

    const FGuid ID = EntityData->GetID();

    // Find and remove from main map
    const TScriptInterface<IEntity>* InterfaceRef = EntityMap.Find(ID);
    if (!InterfaceRef) { return; }

    // Cache values before removal
    const EFaction Faction = Entity->GetFaction();
    const EEntityType Type = Entity->GetEntityType();

    // Lambda for ID comparison - reused across all collections
    auto RemoveByID = [&ID](const TScriptInterface<IEntity>& Item) {
        return Item->GetEntityData() && Item->GetEntityData()->GetID() == ID;
    };

    // Remove from all collections
    if (FEntityArray* FactionArray = FactionMap.Find(Faction)) {
        FactionArray->Entities.RemoveAll(RemoveByID);
    }

    if (FEntityArray* TypeArray = TypeMap.Find(Type)) { TypeArray->Entities.RemoveAll(RemoveByID); }

    const FFactionTypeKey FactionAndType(Faction, Type);
    if (FEntityArray* FactionTypeArray = FactionAndTypeMap.Find(FactionAndType)) {
        FactionTypeArray->Entities.RemoveAll(RemoveByID);
    }

    AllEntities.RemoveAll(RemoveByID);

    // Broadcast removal event before removing from main map
    OnEntityRemoved.Broadcast(*InterfaceRef);

    // Finally remove from main map
    EntityMap.Remove(ID);
}

TScriptInterface<IEntity> UEntityManager::GetEntityByID(const FGuid& ID) const {
    if (const TScriptInterface<IEntity>* Found = EntityMap.Find(ID)) { return *Found; }
    return TScriptInterface<IEntity>();
}

TArray<TScriptInterface<IEntity>> UEntityManager::GetEntitiesByFaction(EFaction Faction) const {
    if (const FEntityArray* Found = FactionMap.Find(Faction)) { return Found->Entities; }
    return TArray<TScriptInterface<IEntity>>();
}

TArray<TScriptInterface<IEntity>> UEntityManager::GetEntitiesByType(EEntityType Type) const {
    if (const FEntityArray* Found = TypeMap.Find(Type)) { return Found->Entities; }
    return TArray<TScriptInterface<IEntity>>();
}

TArray<TScriptInterface<IEntity>> UEntityManager::GetEntitiesByFactionAndType(
    EFaction Faction,
    EEntityType Type) const {
    const FFactionTypeKey FactionAndType(Faction, Type);
    if (const FEntityArray* Found = FactionAndTypeMap.Find(FactionAndType)) {
        return Found->Entities;
    }
    return TArray<TScriptInterface<IEntity>>();
}
