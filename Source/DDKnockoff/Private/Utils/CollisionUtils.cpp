#include "Utils/CollisionUtils.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "Collision/DDCollisionChannels.h"

void UCollisionUtils::SetupHurtbox(UPrimitiveComponent* Component) {
    if (!Component) { return; }

    // Basic hurtbox settings
    Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Component->SetCollisionObjectType(DDCollisionChannels::ECC_Hurtbox); // Hurtbox channel

    // Ignore everything by default
    Component->SetCollisionResponseToAllChannels(ECR_Ignore);

    // Only overlap with damage-dealing objects
    Component->SetCollisionResponseToChannel(DDCollisionChannels::ECC_Hitbox, ECR_Overlap);
    // Attack/Hitbox channel
    Component->SetCollisionResponseToChannel(DDCollisionChannels::ECC_Trigger, ECR_Overlap);

    // No navigation impact
    Component->SetCanEverAffectNavigation(false);

    // No visibility
    Component->SetVisibility(false);

    // Overlap events enabled
    Component->SetGenerateOverlapEvents(true);
}

void UCollisionUtils::SetupAttackHitbox(UPrimitiveComponent* Component) {
    if (!Component) { return; }

    // Basic hurtbox settings
    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Hitbox channel for projectiles
    Component->SetCollisionObjectType(DDCollisionChannels::ECC_Hitbox);

    // Ignore everything by default
    Component->SetCollisionResponseToAllChannels(ECR_Ignore);

    // Overlap with hurtboxes and pawns
    Component->SetCollisionResponseToChannel(DDCollisionChannels::ECC_Hurtbox, ECR_Overlap);
    Component->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Component->SetCollisionResponseToChannel(DDCollisionChannels::ECC_EnemyPawn, ECR_Overlap);

    // No navigation impact
    Component->SetCanEverAffectNavigation(false);

    // Generate overlap events
    Component->SetGenerateOverlapEvents(true);

    // No visibility
    Component->SetVisibility(false);
}

void UCollisionUtils::SetupStructurePhysicalCollision(UPrimitiveComponent* Component) {
    if (!Component) { return; }

    // Basic hurtbox settings
    Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Hitbox channel for projectiles
    Component->SetCollisionObjectType(ECC_WorldDynamic);

    // Ignore everything by default
    Component->SetCollisionResponseToAllChannels(ECR_Block);

    // No navigation impact
    Component->SetCanEverAffectNavigation(false);

    // Generate overlap events
    Component->SetGenerateOverlapEvents(true);

    // No visibility
    Component->SetVisibility(false);
}
