#include "Structures/BowlingBallTurret/BowlingBallTurret.h"

#include "Structures/BowlingBallTurret/BowlingBallTurretAmmo.h"
#include "Structures/BowlingBallTurret/BowlingBallTurretAnimInstance.h"
#include "Structures/Components/EnemyDetectionComponent.h"


// Sets default values
ABowlingBallTurret::ABowlingBallTurret() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Create enemy detection component
    EnemyDetectionComponent = CreateDefaultSubobject<UEnemyDetectionComponent>(
        TEXT("EnemyDetectionComponent"));
}

// Called when the game starts or when spawned
void ABowlingBallTurret::BeginPlay() {
    Super::BeginPlay();
    ensureAlways(BowlingBallTurretAmmoClass != nullptr);

    // Get the anim instance
    AnimInstance = Cast<UBowlingBallTurretAnimInstance>(BaseAnimInstance);
    ensureAlways(AnimInstance != nullptr);
}

TObjectPtr<ABowlingBallTurretAmmo> ABowlingBallTurret::SpawnAmmo() const {
    // Get socket location
    const FTransform SocketTransform = SkeletonMesh->GetSocketTransform(TEXT("ammoSocket"));
    // Spawn the ammo actor at the socket location
    const auto ret = GetWorld()->SpawnActor<ABowlingBallTurretAmmo>(
        BowlingBallTurretAmmoClass,
        SocketTransform.GetLocation(),
        SocketTransform.GetRotation().Rotator());
    return ret;
}

// Called every frame
void ABowlingBallTurret::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    // If we are in preview mode, don't do anything
    if (StructurePlacementState == EStructurePlacementState::Previewing) { return; }

    // Check for enemies
    const FEnemyDetectionResult DetectionResult = EnemyDetectionComponent->DetectEnemies();
    TargetActor = DetectionResult.ClosestEnemy;

    // Update state based on detection result
    CurrentState = DetectionResult.bEnemiesInRange
                       ? EDefensiveStructureState::Attacking
                       : EDefensiveStructureState::Idle;

    // Attack if we have a target
    if (CurrentState == EDefensiveStructureState::Attacking) { Attack(); }

    // Update the anim instance targeting if we have a target
    if (TargetActor) { AnimInstance->PointToTargetLocation(TargetActor->GetActorLocation()); }
    // TODO - return to a neutral position if no enemy detected - or do a 'searching' idle anim.
}

void ABowlingBallTurret::Attack() {
    if (AnimInstance->GetCurrentAnimationState() == EBowlingBallTurretAnimationState::Idle
        && IsAttackOffCooldown()) {
        RecordAttack(); // Start cooldown timer
        AnimInstance->PlayAttackAnim();
    }
}

void ABowlingBallTurret::OnFireNotifyReceived(UAnimSequenceBase* Animation) {
    Super::OnFireNotifyReceived(Animation);

    const auto Ammo = SpawnAmmo();
    Ammo->Fire();
}

void ABowlingBallTurret::ValidateConfiguration() const {
    // Call parent validation first
    Super::ValidateConfiguration();

    // Validate ammo class is set
    ensureAlways(BowlingBallTurretAmmoClass != nullptr);
}
