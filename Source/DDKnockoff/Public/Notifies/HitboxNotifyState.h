#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "HitboxNotifyState.generated.h"

/**
 * Animation notify state for controlling weapon hitbox activation periods.
 * Enables collision detection during specific animation frames for melee attacks.
 * Automatically activates at begin and deactivates at end of the notify window.
 */
UCLASS()
class DDKNOCKOFF_API UHitboxNotifyState : public UAnimNotifyState {
    GENERATED_BODY()

public:
    /**
     * Activate hitbox collision at the start of the notify window.
     * @param MeshComp - Skeletal mesh component playing the animation
     * @param Animation - Animation sequence containing this notify
     * @param TotalDuration - Total duration of the notify state
     * @param EventReference - Reference to the notify event
     */
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp,
                             UAnimSequenceBase* Animation,
                             float TotalDuration,
                             const FAnimNotifyEventReference& EventReference) override;

    /**
     * Deactivate hitbox collision at the end of the notify window.
     * @param MeshComp - Skeletal mesh component playing the animation
     * @param Animation - Animation sequence containing this notify
     * @param EventReference - Reference to the notify event
     */
    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp,
                           UAnimSequenceBase* Animation,
                           const FAnimNotifyEventReference& EventReference) override;
};
