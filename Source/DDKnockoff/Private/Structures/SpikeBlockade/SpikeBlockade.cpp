#include "Structures/SpikeBlockade/SpikeBlockade.h"

#include "Damage/DDDamageType.h"
#include "Damage/DamagePayload.h"


// Sets default values
ASpikeBlockade::ASpikeBlockade() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

void ASpikeBlockade::TakeDamage(const FDamagePayload& DamagePayload) {
    // Super call handles taking the damage
    Super::TakeDamage(DamagePayload);

    // Deal damage to the damage instigator if the damage type is melee
    if (DamagePayload.DamageType == EDDDamageType::Melee) {
        if (IEntity* Entity = Cast<IEntity>(DamagePayload.DamageInstigator.GetObject())) {
            FDamagePayload ResponseDamagePayload;
            ResponseDamagePayload.DamageAmount = 50.0f;
            ResponseDamagePayload.DamageInstigator = this;
            Entity->TakeDamage(ResponseDamagePayload);
        }
    }
}
