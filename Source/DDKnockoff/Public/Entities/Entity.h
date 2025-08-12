#pragma once

#include "CoreMinimal.h"
#include "FactionEnums.h"
#include "EntityTypeEnums.h"
#include "UObject/Interface.h"
#include "Entity.generated.h"

struct FDamagePayload;
class UEntityManager;
class ADDKnockoffGameMode;
class UEntityData;

UINTERFACE(MinimalAPI)
class UEntity : public UInterface {
    GENERATED_BODY()
};

/**
 * Core interface for all interactive entities in the game world.
 * Provides standard behavior for damage, animation notifications, and entity identification.
 * Implemented by characters, structures, and interactive objects.
 */
class DDKNOCKOFF_API IEntity {
    GENERATED_BODY()

public:
    // Core entity properties (required implementations)

    /**
     * Get the entity's data component containing configuration and state.
     * @return Entity data instance
     */
    virtual UEntityData* GetEntityData() const = 0;

    /**
     * Get the faction this entity belongs to for targeting and AI decisions.
     * @return Entity's faction
     */
    virtual EFaction GetFaction() const = 0;

    /**
     * Get the actor instance this entity represents.
     * @return Actor instance
     */
    virtual AActor* GetActor() = 0;

    /**
     * Get the specific type of entity for classification purposes.
     * @return Entity type
     */
    virtual EEntityType GetEntityType() const = 0;

    // Combat and interaction (optional implementations)

    /**
     * Apply damage to this entity.
     * @param DamagePayload - Complete damage information including type, amount, and source
     */
    virtual void TakeDamage(const FDamagePayload& DamagePayload) {}

    /**
     * Apply physics-based knockback to this entity.
     * @param Direction - Normalized direction vector for knockback
     * @param Strength - Force magnitude to apply
     */
    virtual void TakeKnockback(const FVector& Direction, const float Strength) {}

    /**
     * Check if this entity can currently be targeted by other entities.
     * @return true if entity is targetable
     */
    virtual bool IsCurrentlyTargetable() const;

    /**
     * Get the entity's collision half-height for targeting calculations.
     * @return Half-height in world units
     */
    virtual float GetHalfHeight() const { return 0.0f; }

    // Animation callbacks (optional implementations)

    virtual void OnFireNotifyReceived(UAnimSequenceBase* Animation) {}
    virtual void OnHitboxBeginNotifyReceived(UAnimSequenceBase* animSequence) {}
    virtual void OnHitboxEndNotifyReceived(UAnimSequenceBase* animSequence) {}
    virtual void OnAnimNotifyReceived_BlendOut(UAnimSequenceBase* animSequence) {}

    // TODO: Move interaction system to separate interface
    virtual void Interact() {}
};
