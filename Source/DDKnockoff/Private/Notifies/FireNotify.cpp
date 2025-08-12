#include "Notifies/FireNotify.h"
#include "Entities/Entity.h"

void UFireNotify::Notify(USkeletalMeshComponent* MeshComp,
                         UAnimSequenceBase* Animation,
                         const FAnimNotifyEventReference& EventReference) {
    Super::Notify(MeshComp, Animation, EventReference);

    const auto Entity = Cast<IEntity>(MeshComp->GetOwner());
    if (!Entity) { return; }
    Entity->OnFireNotifyReceived(Animation);
}
