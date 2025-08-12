#include "Notifies/HitboxNotifyState.h"
#include "Entities/Entity.h"


void UHitboxNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp,
                                     UAnimSequenceBase* Animation,
                                     float TotalDuration,
                                     const FAnimNotifyEventReference& EventReference) {
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    const auto Entity = Cast<IEntity>(MeshComp->GetOwner());
    if (!Entity) { return; }
    Entity->OnHitboxBeginNotifyReceived(Animation);
}

void UHitboxNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp,
                                   UAnimSequenceBase* Animation,
                                   const FAnimNotifyEventReference& EventReference) {
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    const auto Entity = Cast<IEntity>(MeshComp->GetOwner());
    if (!Entity) { return; }
    Entity->OnHitboxEndNotifyReceived(Animation);
}
