#pragma once

#include "CoreMinimal.h"
#include "SliceAndDiceEnums.h"
#include "Animation/AnimInstance.h"
#include "SliceAndDiceAnimInstance.generated.h"

/**
 * Animation instance for SliceAndDice defensive structures.
 * Manages blade rotation and attack animation progression.
 * Controls visual feedback for spinning blade attack sequences.
 */
UCLASS()
class DDKNOCKOFF_API USliceAndDiceAnimInstance : public UAnimInstance {
    GENERATED_BODY()

public:
    // Blade animation data

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SliceAndDice")
    float BladeAnimationProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SliceAndDice")
    float BladeRotation = 0.0f;

    // Animation state

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    ESliceAndDiceAnimationState CurrentSliceAndDiceAnimationState =
        ESliceAndDiceAnimationState::Idle;
};
