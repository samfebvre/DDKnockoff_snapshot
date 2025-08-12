#pragma once

#include "CoreMinimal.h"
#include "ResourceChest.h"
#include "Animation/AnimInstance.h"
#include "ResourceChestAnimInstance.generated.h"

/**
 * Animation instance for resource chest actors.
 * Handles chest opening animations and state synchronization.
 * Manages both chest open/close state and size scaling state.
 */
UCLASS()
class DDKNOCKOFF_API UResourceChestAnimInstance : public UAnimInstance {
    GENERATED_BODY()

public:
    /**
     * Trigger the chest opening animation sequence.
     */
    void PlayOpeningAnimation();

    // Animation state
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    EChestState CurrentChestState = EChestState::Closed;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest")
    EChestGrowState ChestGrowState = EChestGrowState::FullSize;
};
