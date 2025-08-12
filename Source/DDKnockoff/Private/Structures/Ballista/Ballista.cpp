#include "Structures/Ballista/Ballista.h"

#include "Structures/Ballista/BallistaAmmo.h"
#include "Structures/Ballista/BallistaAnimInstance.h"
#include "Structures/Ballista/BallistaEnums.h"
#include "Structures/Components/EnemyDetectionComponent.h"
#include "Core/DDKnockoffGameSettings.h"
#include "UObject/ConstructorHelpers.h"


// Sets default values
ABallista::ABallista() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Create enemy detection component
    EnemyDetectionComponent = CreateDefaultSubobject<UEnemyDetectionComponent>(
        TEXT("EnemyDetectionComponent"));

    // Load ammo class from game settings
    if (BallistaAmmoClass == nullptr) {
        const UDDKnockoffGameSettings* GameSettings = UDDKnockoffGameSettings::Get();
        if (GameSettings && !GameSettings->BallistaAmmoClass.IsEmpty()) {
            static ConstructorHelpers::FClassFinder<ABallistaAmmo> BallistaAmmoClassFinder(
                *GameSettings->BallistaAmmoClass);
            BallistaAmmoClass = BallistaAmmoClassFinder.Class;
        }
    }
}

// Called when the game starts or when spawned
void ABallista::BeginPlay() {
    Super::BeginPlay();

    // Dependencies are handled by parent class EnsureDependenciesInjected() call
    // Instantiate the initial ammo
    SpawnAmmo();
}

void ABallista::SpawnAmmo() {
    // Only spawn if we don't already have ammo
    // The ballista ammo class is validated before this point.
    if (BallistaAmmoInstance == nullptr) {
        BallistaAmmoInstance = GetWorld()->SpawnActor<ABallistaAmmo>(
            BallistaAmmoClass,
            GetActorLocation(),
            GetActorRotation()
            );

        if (BallistaAmmoInstance) {
            // Attach to skeletal mesh on the "ammoSocket" socket
            BallistaAmmoInstance->AttachToComponent(
                SkeletonMesh,
                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                TEXT("ammoSocket")
                );
        }
    }
}

// Called every frame
void ABallista::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    // If we are in preview mode, don't do anything
    if (StructurePlacementState == EStructurePlacementState::Previewing) { return; }

    UpdateTargeting();
    UpdateAnimationState();

    // Attack if conditions are met
    if (ShouldAttack() && CanAttack()) { Attack(); }
}

void ABallista::Attack() {
    if (CanAttack()) {
        RecordAttack(); // Start cooldown timer
        AnimInstance->PlayAttackAnim();
        // Ensure we have ammo ready for the next shot
        SpawnAmmo();
    }
}

bool ABallista::ShouldAttack() const {
    return CurrentState == EDefensiveStructureState::Attacking && TargetActor != nullptr;
}

bool ABallista::CanAttack() const {
    return AnimInstance->GetCurrentBallistaAnimationState() == EBallistaAnimationState::Idle
           && IsAttackOffCooldown();
}

void ABallista::UpdateTargeting() {
    // Check for enemies using detection component
    const FEnemyDetectionResult DetectionResult = EnemyDetectionComponent->DetectEnemies();
    TargetActor = DetectionResult.ClosestEnemy;

    // Update state based on detection result
    CurrentState = DetectionResult.bEnemiesInRange
                       ? EDefensiveStructureState::Attacking
                       : EDefensiveStructureState::Idle;
}

void ABallista::UpdateAnimationState() const {
    if (TargetActor) { AnimInstance->PointToTargetLocation(TargetActor->GetActorLocation()); }
    // TODO - return to a neutral position if no enemy detected - or do a 'searching' idle anim.
}

void ABallista::OnFireNotifyReceived(UAnimSequenceBase* Animation) {
    Super::OnFireNotifyReceived(Animation);

    if (BallistaAmmoInstance) {
        // Detach and fire the ammo
        BallistaAmmoInstance->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        BallistaAmmoInstance->Fire();
        BallistaAmmoInstance = nullptr;
    }
}

void ABallista::EndPlay(const EEndPlayReason::Type EndPlayReason) {
    // Clean up ammo instance before calling Super::EndPlay
    if (BallistaAmmoInstance) {
        BallistaAmmoInstance->Destroy();
        BallistaAmmoInstance = nullptr;
    }

    Super::EndPlay(EndPlayReason);
}

bool ABallista::HasRequiredDependencies() const {
    // Check parent dependencies first
    if (!Super::HasRequiredDependencies()) { return false; }

    // Check ballista-specific dependencies
    return AnimInstance != nullptr;
}

void ABallista::CollectDependencies() {
    // Call parent to collect base dependencies
    Super::CollectDependencies();

    // Only collect animation instance if not already set (e.g., from manual injection)
    if (!AnimInstance) {
        // Collect animation instance dependency from the skeletal mesh component
        AnimInstance = Cast<UBallistaAnimInstance>(BaseAnimInstance);
    }
}

void ABallista::ValidateConfiguration() const {
    // Call parent to validate base configuration
    Super::ValidateConfiguration();

    // Validate ammo class is set
    ensureAlways(BallistaAmmoClass != nullptr);
}

AActor* ABallista::GetTargetActor() const { return TargetActor; }

UEnemyDetectionComponent* ABallista::GetEnemyDetectionComponent() const {
    return EnemyDetectionComponent;
}
