#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/EngineTypes.h"
#include "CollisionUtils.generated.h"

/**
 * Static utility library for collision setup patterns throughout the game.
 * Provides consistent collision configuration for various game objects.
 */
UCLASS()
class DDKNOCKOFF_API UCollisionUtils : public UBlueprintFunctionLibrary {
    GENERATED_BODY()

public:
    /**
     * Configure collision settings for a structure's hurtbox.
     * Used for detecting damage and interactions with the structure.
     * @param Component - Primitive component to configure as hurtbox
     */
    UFUNCTION(BlueprintCallable,
        Category = "Collision|Structure",
        meta = (Keywords = "collision setup hurtbox damage structure"))
    static void SetupHurtbox(UPrimitiveComponent* Component);

    /**
     * Configure collision settings for an entity's attack hitbox.
     * Used for dealing damage to other entities.
     * @param Component - Primitive component to configure as attack hitbox
     */
    UFUNCTION(BlueprintCallable,
        Category = "Collision|Entity",
        meta = (Keywords = "collision setup attack hitbox damage"))
    static void SetupAttackHitbox(UPrimitiveComponent* Component);

    /**
     * Configure collision settings for a structure's physical collision mesh.
     * Used for blocking movement and navigation.
     * @param Component - Primitive component to configure for physical collision
     */
    UFUNCTION(BlueprintCallable,
        Category = "Collision|Structure",
        meta = (Keywords = "collision setup structure physical blocking"))
    static void SetupStructurePhysicalCollision(UPrimitiveComponent* Component);
};
