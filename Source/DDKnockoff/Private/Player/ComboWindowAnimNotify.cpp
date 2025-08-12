#include "Player/ComboWindowAnimNotify.h"
#include "Player/DDKnockoffCharacter.h"

void UComboWindowBeginNotify::Notify(USkeletalMeshComponent* MeshComp,
                                     UAnimSequenceBase* Animation,
                                     const FAnimNotifyEventReference& EventReference) {
    Super::Notify(MeshComp, Animation, EventReference);

    if (const ADDKnockoffCharacter* Character = Cast<ADDKnockoffCharacter>(MeshComp->GetOwner())) {
        Character->OnComboWindowBegin();
    }
}


void UComboTriggerNotify::Notify(USkeletalMeshComponent* MeshComp,
                                 UAnimSequenceBase* Animation,
                                 const FAnimNotifyEventReference& EventReference) {
    Super::Notify(MeshComp, Animation, EventReference);

    if (ADDKnockoffCharacter* Character = Cast<ADDKnockoffCharacter>(MeshComp->GetOwner())) {
        Character->OnComboTrigger();
    }
}
