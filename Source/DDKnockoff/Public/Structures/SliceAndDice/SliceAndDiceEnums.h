#pragma once

/**
 * Animation state enumeration for SliceAndDice structure behavior.
 */
UENUM(BlueprintType)
enum class ESliceAndDiceAnimationState : uint8 {
    Idle UMETA(DisplayName = "Idle"),
    // Blades are stationary or spinning slowly
    Attacking UMETA(DisplayName = "Attack"),
    // Blades are spinning at attack speed
    MAX UMETA(Hidden)
};

/**
 * Hitbox collision state for SliceAndDice damage detection.
 */
UENUM(BlueprintType)
enum class ESliceAndDiceHitboxState : uint8 {
    Disabled UMETA(DisplayName = "Disabled"),
    // Hitbox collision is off (blades too slow)
    Enabled UMETA(DisplayName = "Enabled"),
    // Hitbox collision is active (blades spinning fast)
    MAX UMETA(Hidden)
};
