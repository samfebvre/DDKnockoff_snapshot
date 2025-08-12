#include "Enemies/DDAICharacter.h"

#include "Enemies/AIAnimInstance.h"
#include "Enemies/DDAIController.h"
#include "Components/BoxComponent.h"
#include "Engine/LocalPlayer.h"
#include "Components/CapsuleComponent.h"
#include "Collision/DDCollisionChannels.h"
#include "Core/ManagerHandlerSubsystem.h"
#include "Currency/CurrencySpawner.h"
#include "Currency/CurrencyUtils.h"
#include "Damage/DamagePayload.h"
#include "Damage/DamageUtils.h"
#include "Entities/EntityData.h"
#include "Entities/EntityManager.h"
#include "Health/HealthComponent.h"
#include "Utils/CollisionUtils.h"
#include "Utils/GeometryUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"

//////////////////////////////////////////////////////////////////////////
// ADDNPC

void ADDAICharacter::OnHitboxOverlap(UPrimitiveComponent* OverlappedComponent,
                                     AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp,
                                     int32 OtherBodyIndex,
                                     bool bFromSweep,
                                     const FHitResult& SweepResult) {
    // If the other actor is ourself, ignore it
    if (OtherActor == this) { return; }

    // Apply melee damage using centralized utility
    UDamageUtils::ApplyDamage(OtherActor, this, 10.0f, 0.0f, EDDDamageType::Melee);
}

void ADDAICharacter::OnDeath() {
    // Spawn currency reward using utility function
    if (CurrencyRewardAmount > 0) {
        UCurrencyUtils::SpawnCurrencyBurstFromActor(
            this,
            CurrencySpawner,
            CurrencyRewardAmount,
            MinimumCrystalCount,
            FVector::UpVector,
            // Spawn upward
            300.0f // Launch speed
            );
    }

    Destroy();
}

void ADDAICharacter::Attack(const AActor& Target) {
    // NOTE - Intentionally NOT checking if we can attack.
    // I am trying out a style of development where the 'Can' check is only done in one place - in the Character class.
    // I'm aiming to avoid long strings of 'can' checks this way.

    FRotator LookAtRotation = (Target.GetActorLocation() - GetActorLocation()).
        Rotation();
    LookAtRotation.Pitch = 0.f;
    LookAtRotation.Roll = 0.f;
    SetActorRotation(LookAtRotation);

    AnimInstance->Attack();
}

bool ADDAICharacter::CanAttack() const { return AnimInstance->CanAttack(); }

void ADDAICharacter::AttackIfAble(AActor& Target) {
    if (!CanAttack()) { return; }
    Attack(Target);
}

ADDAICharacter::ADDAICharacter() {
    // Enable ticking for this character
    PrimaryActorTick.bCanEverTick = true;

    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
    GetCapsuleComponent()->SetCollisionResponseToChannel(DDCollisionChannels::ECC_Trigger,
                                                         ECR_Overlap);
    GetCapsuleComponent()->SetCollisionResponseToChannel(DDCollisionChannels::ECC_Hitbox,
                                                         ECR_Overlap);

    // Don't rotate when the controller rotates. Let that just affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    // GetCharacterMovement()->bUseControllerDesiredRotation = false;

    // Character moves in the direction of input...	
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

    // Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
    // instead of recompiling to adjust them
    GetCharacterMovement()->JumpZVelocity = 700.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

    // Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
    // are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

    // AI setup
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    AIControllerClass = ADDAIController::StaticClass();

    // Health setup
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

    // Currency spawner setup
    CurrencySpawner = CreateDefaultSubobject<UCurrencySpawner>(TEXT("CurrencySpawner"));
    CurrencySpawner->SetupAttachment(RootComponent);

    // Hitbox Setup
    HitboxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Hitbox"));
    HitboxComponent->SetupAttachment(RootComponent);
    UCollisionUtils::SetupAttackHitbox(HitboxComponent);

    // Target Detection Collider
    TargetDetectionCollider = CreateDefaultSubobject<UCapsuleComponent>(
        TEXT("TargetDetectionCollider"));
    TargetDetectionCollider->SetupAttachment(RootComponent);
    TargetDetectionCollider->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    TargetDetectionCollider->SetCollisionObjectType(DDCollisionChannels::ECC_Trigger);
    TargetDetectionCollider->SetCollisionResponseToAllChannels(ECR_Ignore);
    TargetDetectionCollider->SetCollisionResponseToChannel(DDCollisionChannels::ECC_Hurtbox,
                                                           ECR_Overlap);

    // Create entity data
    EntityData = CreateDefaultSubobject<UEntityData>(TEXT("EntityData"));
}


FVector ADDAICharacter::GetCapsuleBottomLocation() const {
    // Get the location of the bottom of the capsule component
    return GetActorLocation() - FVector(0.f,
                                        0.f,
                                        GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
}

CharacterActorOverlapState ADDAICharacter::GetActorOverlapState() const {
    return ActorOverlapState;
}

TWeakObjectPtr<AActor> ADDAICharacter::GetClosestOverlappingStructure() const {
    return ClosestOverlappingStructure;
}

void ADDAICharacter::Tick(const float DeltaSeconds) {
    Super::Tick(DeltaSeconds);
    TargetDetectionCollider->GetOverlappingActors(ActorsInDetectionRange);

    // If we are overlapping any defensive structures, we need to set the overlap state
    AActor* ClosestOverlappingRelevantActor = nullptr;
    float ClosestDistance = MAX_FLT;

    for (const auto Actor : ActorsInDetectionRange) {
        if (const auto Entity = Cast<IEntity>(Actor)) {
            if ((Entity->GetEntityType() == EEntityType::Structure_Defense
                 || Entity->GetEntityType() == EEntityType::Structure_Crystal)
                && Entity->IsCurrentlyTargetable()) {
                // Calculate distance to this actor
                const float Distance = UGeometryUtils::GetDistanceBetweenActors(this, Actor);
                if (Distance < ClosestDistance) {
                    ClosestDistance = Distance;
                    ClosestOverlappingRelevantActor = Actor;
                }
            }
        }
    }

    if (ClosestOverlappingRelevantActor == nullptr) {
        // If we are not overlapping any relevant actors, set the overlap state to none
        ActorOverlapState = CharacterActorOverlapState::None;
    } else {
        ActorOverlapState = CharacterActorOverlapState::OverlappingStructure;
        ClosestOverlappingStructure = ClosestOverlappingRelevantActor;
    }
}

EEnemyPoseState ADDAICharacter::GetCurrentPoseState() const {
    return AnimInstance->GetCurrentPoseState();
}

void ADDAICharacter::TakeDamage(const FDamagePayload& DamagePayload) {
    HealthComponent->TakeDamage(DamagePayload.DamageAmount);
}

void ADDAICharacter::OnHitboxBeginNotifyReceived(UAnimSequenceBase* Animation) {
    HitboxComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ADDAICharacter::OnHitboxEndNotifyReceived(UAnimSequenceBase* Animation) {
    HitboxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ADDAICharacter::TakeKnockback(const FVector& Direction, const float Strength) {
    LaunchCharacter(Direction * Strength, true, true);
    EnterHitReaction();
    Evt_OnTookKnockback.Broadcast();
}

EFaction ADDAICharacter::GetFaction() const { return EFaction::Enemy; }
AActor* ADDAICharacter::GetActor() { return this; }

float ADDAICharacter::GetHalfHeight() const {
    return GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

void ADDAICharacter::EnterHitReaction() const {
    AnimInstance->EnterHitReaction();
    GetCharacterMovement()->StopMovementImmediately();
}

void ADDAICharacter::BeginPlay() {
    Super::BeginPlay();

    // Handle dependency injection with single call
    EnsureDependenciesInjected();

    // Validate configuration
    ValidateConfiguration();

    // Register with EntityManager using injected dependency
    EntityManager->RegisterEntity(this);

    // Initialize animation instance
    AnimInstance = Cast<UAIAnimInstance>(GetMesh()->GetAnimInstance());
    // Assert that the anim instance is valid - ie. that the anim instance is of the correct type
    ensureAlways(AnimInstance);

    HealthComponent->OnReachedZeroHealth.RemoveDynamic(this, &ADDAICharacter::OnDeath);
    HealthComponent->OnReachedZeroHealth.AddDynamic(this, &ADDAICharacter::OnDeath);
    HitboxComponent->OnComponentBeginOverlap.RemoveDynamic(this, &ADDAICharacter::OnHitboxOverlap);
    HitboxComponent->OnComponentBeginOverlap.AddDynamic(this, &ADDAICharacter::OnHitboxOverlap);
}

void ADDAICharacter::EndPlay(const EEndPlayReason::Type EndPlayReason) {
    // Unregister from EntityManager using injected dependency
    EntityManager->UnregisterEntity(this);
    Super::EndPlay(EndPlayReason);
}

// IDependencyInjectable interface implementation
bool ADDAICharacter::HasRequiredDependencies() const { return EntityManager != nullptr; }

void ADDAICharacter::CollectDependencies() {
    // Get EntityManager from world subsystem (normal gameplay fallback)
    if (!EntityManager) {
        EntityManager = UManagerHandlerSubsystem::GetManager<UEntityManager>(GetWorld());
    }
}

// IConfigurationValidatable interface implementation
void ADDAICharacter::ValidateConfiguration() const {
    // Validate components
    ensureAlways(HealthComponent != nullptr);
    ensureAlways(HitboxComponent != nullptr);
    ensureAlways(TargetDetectionCollider != nullptr);
    ensureAlways(CurrencySpawner != nullptr);

    // Validate currency settings
    ensureAlways(CurrencyRewardAmount >= 0);
    ensureAlways(MinimumCrystalCount >= 0);

    // Validate mesh and animation
    ensureAlways(GetMesh() != nullptr);
    ensureAlways(GetMesh()->GetAnimClass() != nullptr);
}
