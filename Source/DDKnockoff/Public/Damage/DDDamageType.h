#pragma once

/**
 * Enumeration of damage types used for combat categorization and potential resistance systems.
 * Used by damage dealing systems to classify different attack methods.
 */
UENUM(BlueprintType, meta = (ScriptName = "DDDamageType"))
enum class EDDDamageType : uint8 {
    None UMETA(DisplayName = "None"),
    // No damage type specified
    Melee UMETA(DisplayName = "Melee"),
    // Close-range physical attacks
    Ranged UMETA(DisplayName = "Ranged"),
    // Projectile and distance attacks
    Special UMETA(DisplayName = "Special"),
    // Unique or magical damage types

    MAX UMETA(Hidden)
};
