#pragma once

#include "CoreMinimal.h"
#include "EntityTypeEnums.generated.h"

/**
 * Classification system for different entity types in the game world.
 * Used by entity manager for organization and gameplay systems for type-specific logic.
 */
UENUM(BlueprintType)
enum class EEntityType : uint8 {
    None UMETA(DisplayName = "None"),
    // No type assigned
    Character UMETA(DisplayName = "Character"),
    // AI enemies and NPCs
    Structure_Defense UMETA(DisplayName = "Structure"),
    // Defensive turrets and blockades
    Structure_Crystal UMETA(DisplayName = "Crystal"),
    // Currency crystal structures
    Projectile UMETA(DisplayName = "Projectile"),
    // Thrown or fired projectiles
    Interactable UMETA(DisplayName = "Prop"),
    // Interactive objects and props

    MAX UMETA(Hidden)
};
