#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ComboWindowAnimNotify.generated.h"

/**
 * Animation Notify to mark the beginning of a combo input window
 */
UCLASS()
class DDKNOCKOFF_API UComboWindowBeginNotify : public UAnimNotify {
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp,
                        UAnimSequenceBase* Animation,
                        const FAnimNotifyEventReference& EventReference) override;
};


/**
 * Animation Notify to trigger execution of buffered combo input
 */
UCLASS()
class DDKNOCKOFF_API UComboTriggerNotify : public UAnimNotify {
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp,
                        UAnimSequenceBase* Animation,
                        const FAnimNotifyEventReference& EventReference) override;
};
