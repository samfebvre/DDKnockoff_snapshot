#pragma once

#include "CoreMinimal.h"
#include "BallistaEnums.h"
#include "BallistaAnimationInterface.h"
#include "Animation/AnimInstance.h"
#include "BallistaAnimInstance.generated.h"

/**
 * Animation instance for Ballista defensive structures.
 * Implements IBallistaAnimationInterface for testable targeting and attack animations.
 * Handles smooth rotation and projectile firing animation sequences.
 */
UCLASS()
class DDKNOCKOFF_API UBallistaAnimInstance
    : public UAnimInstance, public IBallistaAnimationInterface {
    GENERATED_BODY()

public:
    // UAnimInstance Interface
    virtual void NativeBeginPlay() override;

    // IBallistaAnimationInterface Implementation
    virtual void PointToTargetLocation(const FVector& TargetLocation) override;
    virtual void PlayAttackAnim() override;

    virtual EBallistaAnimationState GetCurrentBallistaAnimationState() const override {
        return CurrentBallistaAnimationState;
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
    EBallistaAnimationState CurrentBallistaAnimationState = EBallistaAnimationState::Idle;
};
