#pragma once

#include "CoreMinimal.h"
#include "BouncerBlockadeEnums.h"
#include "Animation/AnimInstance.h"
#include "BouncerBlockadeAnimInstance.generated.h"

/**
 * Animation instance for BouncerBlockade defensive structures.
 * Manages animation states between idle and attacking behavior.
 * Coordinates visual feedback for bounce attack sequences.
 */
UCLASS()
class DDKNOCKOFF_API UBouncerBlockadeAnimInstance : public UAnimInstance {
    GENERATED_BODY()

public:
    // Animation configuration

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    UAnimMontage* AttackMontage;

    // Animation state

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    EBouncerBlockadeAnimationState CurrentBouncerBlockadeAnimationState =
        EBouncerBlockadeAnimationState::Idle;
};
