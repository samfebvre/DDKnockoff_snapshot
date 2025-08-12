#pragma once

/**
 * Animation state enumeration for BouncerBlockade structure behavior.
 */
UENUM(BlueprintType)
enum class EBouncerBlockadeAnimationState : uint8 {
    Idle UMETA(DisplayName = "Idle"),
    // Structure is idle, waiting for enemies
    Attacking UMETA(DisplayName = "Attack"),
    // Structure is performing bounce attack
    MAX UMETA(Hidden)
};
