#include "Player/PlayerInteractionComponent.h"

#include "Components/SphereComponent.h"
#include "Collision/DDCollisionChannels.h"
#include "Entities/Entity.h"


// Sets default values for this component's properties
UPlayerInteractionComponent::UPlayerInteractionComponent() {
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    InteractionDetectionSphere = CreateDefaultSubobject<USphereComponent>(
        TEXT("InteractionDetectionSphere"));
    InteractionDetectionSphere->SetCanEverAffectNavigation(false);
}


// Called when the game starts
void UPlayerInteractionComponent::BeginPlay() { Super::BeginPlay(); }


// Called every frame
void UPlayerInteractionComponent::TickComponent(float DeltaTime,
                                                ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateInteractableInformation();
    InteractWithAllInteractablesInRange();
}

void UPlayerInteractionComponent::OnRegister() {
    Super::OnRegister();

    InteractionDetectionSphere->SetupAttachment(GetOwner()->GetRootComponent());
    InteractionDetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    InteractionDetectionSphere->SetCollisionObjectType(DDCollisionChannels::ECC_Interact);
    InteractionDetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionDetectionSphere->SetCollisionResponseToChannel(
        DDCollisionChannels::ECC_Interact,
        ECR_Overlap);
}

void UPlayerInteractionComponent::PopulateInteractableResultData() {
    InteractionDetectionResult.InteractablesInRange.Empty();
    InteractionDetectionResult.ClosestInteractable = nullptr;
    float ClosestDistance = FLT_MAX;

    for (AActor* Actor : ActorsInRange) {
        if (Actor == GetOwner()) { continue; }

        if (const IEntity* Entity = Cast<IEntity>(Actor)) {
            if (Entity->GetEntityType() == EEntityType::Interactable) {
                InteractionDetectionResult.InteractablesInRange.Add(Actor);
                const float Distance = FVector::DistSquared(Actor->GetActorLocation(),
                                                            GetOwner()->GetActorLocation());
                if (Distance < ClosestDistance) {
                    ClosestDistance = Distance;
                    InteractionDetectionResult.ClosestInteractable = Actor;
                }
            }
        }
    }
}

void UPlayerInteractionComponent::UpdateInteractableInformation() {
    // Get overlapping actors
    ActorsInRange.Empty();
    InteractionDetectionSphere->GetOverlappingActors(ActorsInRange);

    // Check for interactables
    PopulateInteractableResultData();
}

// TODO - desperately needs refactoring
void UPlayerInteractionComponent::InteractWithAllInteractablesInRange() {
    for (AActor* Actor : InteractionDetectionResult.InteractablesInRange) {
        if (IEntity* Entity = Cast<IEntity>(Actor)) { Entity->Interact(); }
    }
}
