#pragma once

#include "CoreMinimal.h"
#include "Structures/DefensiveStructure.h"
#include "SpikeBlockade.generated.h"

/**
 * Passive defensive structure with spike damage on contact.
 * Damages enemies that come into contact with the structure.
 * Provides area denial through collision-based damage.
 */
UCLASS()
class DDKNOCKOFF_API ASpikeBlockade : public ADefensiveStructure {
    GENERATED_BODY()

public:
    ASpikeBlockade();

    // IEntity Interface
    virtual void TakeDamage(const FDamagePayload& DamagePayload) override;
};
