#include "Damage/DamageUtils.h"
#include "Entities/Entity.h"
#include "Engine/Engine.h"

bool UDamageUtils::ApplyDamage(AActor* Target,
                               AActor* Instigator,
                               float DamageAmount,
                               float KnockbackStrength,
                               EDDDamageType DamageType) {
    if (!CanDamageTarget(Target, Instigator)) { return false; }

    // Create damage payload with knockback included
    const FDamagePayload DamagePayload = CreateDamagePayload(
        DamageAmount,
        Instigator,
        KnockbackStrength,
        DamageType);

    // Apply knockback first if needed (to avoid null reference issues if entity is destroyed by damage)
    if (KnockbackStrength > 0.0f) {
        const FVector KnockbackDirection = CalculateKnockbackDirection(Target, Instigator);
        ApplyKnockbackInternal(Target, KnockbackDirection, KnockbackStrength);
    }

    // Apply damage
    return ApplyDamageInternal(Target, DamagePayload);
}

bool UDamageUtils::CanDamageTarget(AActor* Target, AActor* Instigator) {
    // Null checks
    if (!Target || !Instigator) { return false; }

    // Self-damage protection
    if (Target == Instigator) { return false; }

    // Check if target is an entity (only entities can take damage in this system)
    const IEntity* TargetEntity = Cast<IEntity>(Target);
    if (!TargetEntity) { return false; }

    // Check if instigator is an entity (for faction checking)
    const IEntity* InstigatorEntity = Cast<IEntity>(Instigator);
    if (!InstigatorEntity) { return false; }

    // Faction check - cannot damage same faction
    if (TargetEntity->GetFaction() == InstigatorEntity->GetFaction()) { return false; }

    return true;
}

bool UDamageUtils::AreDifferentFactions(AActor* Actor1, AActor* Actor2) {
    if (!Actor1 || !Actor2) { return false; }

    const IEntity* Entity1 = Cast<IEntity>(Actor1);
    const IEntity* Entity2 = Cast<IEntity>(Actor2);

    if (!Entity1 || !Entity2) { return false; }

    return Entity1->GetFaction() != Entity2->GetFaction();
}

FVector UDamageUtils::CalculateKnockbackDirection(AActor* Target,
                                                  AActor* Source,
                                                  float UpwardComponent) {
    if (!Target || !Source) { return FVector::ZeroVector; }

    // Calculate direction from source to target
    FVector Direction = (Target->GetActorLocation() - Source->GetActorLocation()).GetSafeNormal();

    // Remove Z component and normalize
    Direction.Z = 0.0f;
    Direction.Normalize();

    // Add upward component
    Direction.Z += UpwardComponent;

    return Direction;
}

FDamagePayload UDamageUtils::CreateDamagePayload(float DamageAmount,
                                                 AActor* Instigator,
                                                 float KnockbackStrength,
                                                 EDDDamageType DamageType) {
    FDamagePayload DamagePayload;
    DamagePayload.DamageAmount = DamageAmount;
    DamagePayload.KnockbackStrength = KnockbackStrength;

    // Convert AActor* to TScriptInterface<IEntity>
    if (Instigator) {
        IEntity* InstigatorEntity = Cast<IEntity>(Instigator);
        if (InstigatorEntity) {
            DamagePayload.DamageInstigator.SetInterface(InstigatorEntity);
            DamagePayload.DamageInstigator.SetObject(Instigator);
        }
    }

    DamagePayload.DamageType = DamageType;
    return DamagePayload;
}

bool UDamageUtils::ApplyDamageInternal(AActor* Target, const FDamagePayload& DamagePayload) {
    IEntity* TargetEntity = Cast<IEntity>(Target);
    if (!TargetEntity) { return false; }

    TargetEntity->TakeDamage(DamagePayload);
    return true;
}

bool UDamageUtils::ApplyKnockbackInternal(AActor* Target,
                                          const FVector& Direction,
                                          float Strength) {
    IEntity* TargetEntity = Cast<IEntity>(Target);
    if (!TargetEntity) { return false; }

    // Only apply knockback if strength is greater than 0
    if (Strength > 0.0f) { TargetEntity->TakeKnockback(Direction, Strength); }

    return true;
}
