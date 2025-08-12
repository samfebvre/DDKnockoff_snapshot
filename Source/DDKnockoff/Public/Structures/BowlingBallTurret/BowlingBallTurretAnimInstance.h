#pragma once

#include "CoreMinimal.h"
#include "BowlingBallTurretEnums.h"
#include "Animation/AnimInstance.h"
#include "BowlingBallTurretAnimInstance.generated.h"

/**
 * Animation instance for BowlingBallTurret defensive structures.
 * Handles smooth turret rotation and targeting animations.
 * Manages attack animation sequences and rotation smoothing.
 */
UCLASS()
class DDKNOCKOFF_API UBowlingBallTurretAnimInstance : public UAnimInstance {
    GENERATED_BODY()

public:
    // UAnimInstance Interface
    virtual void NativeBeginPlay() override;

    // Targeting system

    /**
     * Smoothly rotate turret to point at target location.
     * @param TargetLocation - World position to aim at
     */
    void PointToTargetLocation(const FVector& TargetLocation);

    // Attack system

    /**
     * Play the turret attack animation montage.
     */
    void PlayAttackAnim();

    /**
     * Get the current animation state.
     * @return Current animation state
     */
    EBowlingBallTurretAnimationState GetCurrentAnimationState() const {
        return CurrentBowlingBallTurretAnimationState;
    }

protected:
    // Montage event handling

    /**
     * Handle montage blending out events for attack cleanup.
     * @param Montage - Montage that is blending out
     * @param bInterrupted - Whether the montage was interrupted
     */
    UFUNCTION()
    void OnMontageBlendingOutStarted(UAnimMontage* Montage, bool bInterrupted);

    // Rotation configuration

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
    double RotationSmoothTime = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rotation")
    double RotationMaxSpeed = 1000;

    // Animation configuration

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TObjectPtr<UAnimMontage> AttackMontage;

    // Runtime state

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    FRotator HeadRotation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    FRotator BodyRotation;

    FRotator CurrentHeadRotationVelocity;
    FRotator CurrentBodyRotationVelocity;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    EBowlingBallTurretAnimationState CurrentBowlingBallTurretAnimationState =
        EBowlingBallTurretAnimationState::Idle;
};
