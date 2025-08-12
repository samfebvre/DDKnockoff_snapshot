#include "Structures/Components/EnemyDetectionComponent.h"

#include "Collision/DDCollisionChannels.h"
#include "Entities/Entity.h"
#include "Utils/GeometryUtils.h"

UEnemyDetectionComponent::UEnemyDetectionComponent() {
    PrimaryComponentTick.bCanEverTick = false;
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("EnemyDetectionSphere"));
    DetectionSphere->SetCanEverAffectNavigation(false);
}

void UEnemyDetectionComponent::BeginPlay() {
    Super::BeginPlay();
    ValidateConfiguration();
}

FEnemyDetectionResult UEnemyDetectionComponent::DetectEnemies() {
    FEnemyDetectionResult Result;
    DetectionSphere->GetOverlappingActors(ActorsInRange);
    Result.bEnemiesInRange = CheckForEnemiesInRange(Result.ClosestEnemy);
    return Result;
}

bool UEnemyDetectionComponent::CheckForEnemiesInRange(TObjectPtr<AActor>& OutClosestEnemy) const {
    OutClosestEnemy = nullptr;
    float ClosestDistance = FLT_MAX;
    bool bEnemiesInRange = false;
    const EFaction OwnerFaction = GetOwnerFaction();

    for (AActor* Actor : ActorsInRange) {
        if (Actor == GetOwner()) { continue; }

        if (const IEntity* Entity = Cast<IEntity>(Actor)) {
            if (Entity->GetFaction() != OwnerFaction && IsActorInVisionCone(Actor)) {
                bEnemiesInRange = true;
                const float Distance = UGeometryUtils::GetDistanceBetweenActors(Actor, GetOwner());
                if (Distance < ClosestDistance) {
                    ClosestDistance = Distance;
                    OutClosestEnemy = Actor;
                }
            }
        }
    }

    return bEnemiesInRange;
}

EFaction UEnemyDetectionComponent::GetOwnerFaction() const {
    if (const IEntity* Entity = Cast<IEntity>(GetOwner())) { return Entity->GetFaction(); }
    return EFaction::None;
}

void UEnemyDetectionComponent::OnRegister() {
    Super::OnRegister();

    DetectionSphere->SetupAttachment(GetOwner()->GetRootComponent());
    DetectionSphere->SetSphereRadius(DetectionRadius);
    DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DetectionSphere->SetCollisionObjectType(DDCollisionChannels::ECC_Trigger);
    DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    DetectionSphere->SetCollisionResponseToChannel(DDCollisionChannels::ECC_Hurtbox, ECR_Overlap);
    DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    DetectionSphere->SetCollisionResponseToChannel(DDCollisionChannels::ECC_EnemyPawn, ECR_Overlap);
}

bool UEnemyDetectionComponent::IsActorInVisionCone(AActor* Actor) const {
    if (!Actor || !GetOwner()) { return false; }

    return UGeometryUtils::IsActorInVisionCone(GetOwner(), Actor, VisionConeAngleDegrees);
}

// IConfigurationValidatable interface implementation
void UEnemyDetectionComponent::ValidateConfiguration() const {
    ensureAlways(DetectionRadius > 0.0f);
    ensureAlways(VisionConeAngleDegrees > 0.0f && VisionConeAngleDegrees <= 360.0f);
}
