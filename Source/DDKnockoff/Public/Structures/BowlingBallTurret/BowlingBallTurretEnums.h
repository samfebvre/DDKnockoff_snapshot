#pragma once

/**
 * Animation state enumeration for BowlingBallTurret structure behavior.
 */
UENUM(BlueprintType)
enum class EBowlingBallTurretAnimationState : uint8 {
    Idle UMETA(DisplayName = "Idle"),
    // Turret is idle, scanning for targets
    Attacking UMETA(DisplayName = "Attack"),
    // Turret is aiming and firing at target
    MAX UMETA(Hidden)
};
