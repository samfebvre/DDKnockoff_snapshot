#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "FireNotify.generated.h"

/**
 * Animation notify for triggering projectile firing or weapon discharge.
 * Used to synchronize weapon firing with animation timing.
 * Typically placed at the precise moment weapons should release projectiles.
 */
UCLASS()
class DDKNOCKOFF_API UFireNotify : public UAnimNotify {
    GENERATED_BODY()

public:
    /**
     * Trigger firing event at the specified animation frame.
     * @param MeshComp - Skeletal mesh component playing the animation
     * @param Animation - Animation sequence containing this notify
     * @param EventReference - Reference to the notify event
     */
    virtual void Notify(USkeletalMeshComponent* MeshComp,
                        UAnimSequenceBase* Animation,
                        const FAnimNotifyEventReference& EventReference) override;
};
