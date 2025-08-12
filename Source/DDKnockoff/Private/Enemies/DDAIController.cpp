#include "Enemies/DDAIController.h"

#include "Enemies/DDAICharacter.h"
#include "Enemies/EnemyCharacterEnums.h"
#include "NavigationSystem.h"
#include "Core/ManagerHandlerSubsystem.h"
#include "Crystal/CrystalStructure.h"
#include "Entities/EntityManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Navigation/PathFollowingComponent.h"

ADDAIController::ADDAIController(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass(
          TEXT("PathFollowingComponent"),
          UCrowdFollowingComponent::StaticClass())), AICharacter(nullptr), CurrentAIState() {
    PrimaryActorTick.bCanEverTick = true;

    if (const auto CrowdFollowingComponent = Cast<
        UCrowdFollowingComponent>(GetPathFollowingComponent())) {
        CrowdFollowingComponent->SetCrowdAvoidanceQuality(ECrowdAvoidanceQuality::Low);
    }
}

void ADDAIController::BeginPlay() {
    Super::BeginPlay();
    
    ValidateConfiguration();

    CurrentAIState = EEnemyAIState::None;
    LastTargetUpdateTime = 0.0f;
}

void ADDAIController::Tick(const float DeltaTime) {
    Super::Tick(DeltaTime);

    if (!CanMakeDecisions()) { return; }

    if (ShouldUpdateTargeting()) {
        UpdateTarget();
        LastTargetUpdateTime = GetWorld()->GetTimeSeconds();
    }

    // TODO - instead of returning, have a loop that runs until the state machine doesn't have anything left to process.
    // Allow external factors to re-run the state machine as necessary.
    ProcessCurrentState();
}

void ADDAIController::OnCharacterTookKnockback() {
    StopMovement();
    // TODO - should prompt a target update after a delay?
    TargetStructure.Reset();
}

void ADDAIController::ProcessCurrentState() {
    switch (CurrentAIState) {
        case EEnemyAIState::None:
            HandleIdleState();
            break;
        case EEnemyAIState::MovingTowardsTarget:
            HandleMovingState();
            break;
        case EEnemyAIState::AttackingTarget:
            HandleAttackingState();
            break;
        default: ;
    }
}

void ADDAIController::HandleIdleState() { TransitionToState(EEnemyAIState::MovingTowardsTarget); }

void ADDAIController::HandleMovingState() {
    // First check if we need a new target
    if (!TargetIsValid()) {
        StopPathingAndMovement();
        UpdateTarget();
        MoveToTarget();
        return;
    }

    // If we're overlapping a defense, prioritize attacking it
    if (CharacterIsOverlappingADefense()) {
        // Update our current target to the closest overlapping structure
        if (AICharacter->GetClosestOverlappingStructure() != TargetStructure) { UpdateTarget(); }

        StopPathingAndMovement();
        TransitionToState(EEnemyAIState::AttackingTarget);
        return;
    }

    if (CheckIfPathComponentIsIdle()) { MoveToTarget(); }
}

void ADDAIController::HandleAttackingState() {
    // If we lost our target or it's being destroyed, find a new one
    if (!TargetIsValid()) {
        TransitionToState(EEnemyAIState::MovingTowardsTarget);
        return;
    }

    // If we're overlapping a defense, prioritize attacking it
    if (CharacterIsOverlappingADefense()) {
        // Update our current target to the closest overlapping structure
        if (AICharacter->GetClosestOverlappingStructure() != TargetStructure) { UpdateTarget(); }

        AttackIfAble(*TargetStructure.Get());
        return;
    }

    TransitionToState(EEnemyAIState::MovingTowardsTarget);
}

bool ADDAIController::CharacterIsOverlappingADefense() const {
    if (!AICharacter) { return false; }

    return AICharacter->GetActorOverlapState() == CharacterActorOverlapState::OverlappingStructure;
}

void ADDAIController::TransitionToState(const EEnemyAIState NewState) {
    if (CurrentAIState == NewState) { return; }

    // TODO - maybe want to do the movement stopping when we enter states via a state machine instead of every frame.
    CurrentAIState = NewState;
}

bool ADDAIController::CanMakeDecisions() const {
    if (!AICharacter) { return false; }

    return AICharacter->GetCurrentPoseState() == EEnemyPoseState::Locomotion;
}

bool ADDAIController::ShouldUpdateTargeting() const {
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    return CurrentTime - LastTargetUpdateTime >= TargetUpdateInterval;
}

void ADDAIController::AttackIfAble(AActor& Target) const {
    if (AICharacter) { AICharacter->AttackIfAble(Target); }
}

void ADDAIController::StopPathingAndMovement() {
    if (GetPathFollowingComponent()->GetStatus() == EPathFollowingStatus::Moving) {
        StopMovement();
        if (AICharacter) { AICharacter->GetCharacterMovement()->StopActiveMovement(); }
    }
}

void ADDAIController::MoveToTarget() {
    if (!TargetIsValid()) { return; }

    FAIMoveRequest MoveRequest;
    MoveRequest.SetGoalActor(Cast<IEntity>(TargetStructure.Get())->GetActor());
    MoveRequest.SetAcceptanceRadius(AcceptanceRadius);

    FNavPathSharedPtr NavPath;
    MoveTo(MoveRequest, &NavPath);
}

// TODO - This is extremely computationally expensive and should be replaced when we overhaul the AI system.
void ADDAIController::UpdateTarget() {
    if (CharacterIsOverlappingADefense()) {
        TargetStructure = AICharacter->GetClosestOverlappingStructure();
        return;
    }

    // TODO - should cache the entity manager instead of getting it every time.
    const UEntityManager* EntityManager = UManagerHandlerSubsystem::GetManager<
        UEntityManager>(GetWorld());
    auto Crystals = EntityManager->GetEntitiesByType(EEntityType::Structure_Crystal);

    ACrystalStructure* NearestCrystal = nullptr;
    float SmallestPathCost = FLT_MAX;

    for (int i = 0; i < Crystals.Num(); ++i) {
        const auto CrystalEntity = Crystals[i];
        if (!CrystalEntity.GetObject() || !IsValid(CrystalEntity.GetObject())) { continue; }

        ACrystalStructure* Crystal = Cast<ACrystalStructure>(CrystalEntity->GetActor());

        Chaos::FReal PathCost = 0.0f;
        const UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(
            GetWorld());

        NavSys->GetPathCost(
            GetPawn()->GetActorLocation(),
            Crystal->GetActorLocation(),
            PathCost);

        if (PathCost < SmallestPathCost) {
            SmallestPathCost = PathCost;
            NearestCrystal = Crystal;
        }
    }

    TargetStructure = NearestCrystal;
}

bool ADDAIController::TargetIsValid() const {
    return TargetStructure.IsValid() && TargetStructure->GetClass()->ImplementsInterface(
               UEntity::StaticClass());
}

bool ADDAIController::CheckIfPathComponentIsIdle() const {
    return GetPathFollowingComponent()->GetStatus() == EPathFollowingStatus::Idle;
}

void ADDAIController::OnPossess(APawn* InPawn) {
    Super::OnPossess(InPawn);

    AICharacter = Cast<ADDAICharacter>(GetPawn());
    ensureAlways(AICharacter != nullptr);

    AICharacter->Evt_OnTookKnockback.
                 RemoveDynamic(this, &ADDAIController::OnCharacterTookKnockback);
    AICharacter->Evt_OnTookKnockback.AddDynamic(this, &ADDAIController::OnCharacterTookKnockback);
}

// IConfigurationValidatable interface implementation
void ADDAIController::ValidateConfiguration() const {
    ensureAlways(AcceptanceRadius > 0.0f);
    ensureAlways(TargetUpdateInterval > 0.0f);
}
