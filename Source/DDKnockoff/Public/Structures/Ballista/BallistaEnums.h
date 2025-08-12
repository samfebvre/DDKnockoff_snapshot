#pragma once

/**
 * Animation state enumeration for Ballista structure behavior.
 */
UENUM(BlueprintType)
enum class EBallistaAnimationState : uint8 {
    Idle UMETA(DisplayName = "Idle"),
    // Ballista is idle, tracking targets
    Attacking UMETA(DisplayName = "Attack"),
    // Ballista is aiming and firing projectile

    MAX UMETA(Hidden)
};
