#pragma once

#include "DDDamageType.h"
#include "DamagePayload.generated.h"

class IEntity;

/**
 * Complete data package for damage transactions between entities.
 * Contains all information needed for damage processing, knockback, and source tracking.
 */
USTRUCT(BlueprintType)
struct FDamagePayload {
    GENERATED_BODY()

    /** Amount of damage to apply */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float DamageAmount = 0.0f;

    /** Entity responsible for causing this damage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    TScriptInterface<IEntity> DamageInstigator = nullptr;

    /** Type classification for this damage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    EDDDamageType DamageType = EDDDamageType::None;

    /** Physics force to apply for knockback effects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
    float KnockbackStrength = 0.0f;
};
