#include "Structures/SliceAndDice/SliceAndDice.h"

#include "Structures/SliceAndDice/SliceAndDiceAnimInstance.h"
#include "Damage/DamageUtils.h"
#include "Utils/CollisionUtils.h"
#include "Structures/Components/EnemyDetectionComponent.h"

// Sets default values
ASliceAndDice::ASliceAndDice() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Setup the hitbox mesh
    HitboxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HitboxMesh"));
    HitboxMesh->SetupAttachment(RootComponent);
    HitboxMesh->SetVisibility(false);
    UCollisionUtils::SetupAttackHitbox(HitboxMesh);

    // Create enemy detection component
    EnemyDetectionComponent = CreateDefaultSubobject<UEnemyDetectionComponent>(
        TEXT("EnemyDetectionComponent"));
}

// Called when the game starts or when spawned
void ASliceAndDice::BeginPlay() {
    Super::BeginPlay();

    // Get the anim instance
    AnimInstance = Cast<USliceAndDiceAnimInstance>(BaseAnimInstance);
    ensureAlways(AnimInstance != nullptr);
}

// Called every frame
void ASliceAndDice::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    if (StructurePlacementState == EStructurePlacementState::Previewing) { return; }

    UpdateStructureState();
    UpdateBladeRotation(DeltaTime);
    HandleHitDetection();
}

void ASliceAndDice::UpdateStructureState() {
    const FEnemyDetectionResult DetectionResult = EnemyDetectionComponent->DetectEnemies();
    CurrentState = DetectionResult.bEnemiesInRange
                       ? EDefensiveStructureState::Attacking
                       : EDefensiveStructureState::Idle;
}

void ASliceAndDice::UpdateBladeRotation(float DeltaTime) {
    const float DirectionMultiplier = CurrentState == EDefensiveStructureState::Attacking
                                          ? 1.0f
                                          : -1.0f;

    // Update rotation velocity
    CurrentBladeRotationVelocity += BladeRotationAcceleration * DeltaTime * DirectionMultiplier;
    CurrentBladeRotationVelocity = FMath::Clamp(CurrentBladeRotationVelocity,
                                                0.0f,
                                                BladeRotationMaxSpeed);

    // Update rotation angle
    BladeRotation += CurrentBladeRotationVelocity * DeltaTime;
    BladeRotation = FMath::Fmod(BladeRotation, 360.0f);

    UpdateBladeAnimation();
    UpdateHitboxScale();
    UpdateHitboxState();
}

void ASliceAndDice::UpdateBladeAnimation() const {
    const float BladeAnimProgress = FMath::Clamp(
        CurrentBladeRotationVelocity / BladeRotationMaxSpeed,
        0.0f,
        1.0f);
    AnimInstance->BladeAnimationProgress = BladeAnimProgress;
    AnimInstance->BladeRotation = BladeRotation;
}

void ASliceAndDice::UpdateHitboxScale() const {
    const float BladeValue = CurrentBladeRotationVelocity / BladeRotationMaxSpeed;
    const float Angle = FMath::Lerp(0.0f, 90.0f, BladeValue);
    const float SinAngle = FMath::Sin(FMath::DegreesToRadians(Angle));
    const float Scale = BladeRadius * SinAngle + 1;

    HitboxMesh->SetRelativeScale3D(FVector(Scale, Scale, 1.0f));
}

void ASliceAndDice::UpdateHitboxState() {
    const float BladeValue = CurrentBladeRotationVelocity / BladeRotationMaxSpeed;
    const bool ShouldBeEnabled = BladeValue > HitboxEnabledThreshold;

    if (ShouldBeEnabled && HitboxState == ESliceAndDiceHitboxState::Disabled) {
        HitboxState = ESliceAndDiceHitboxState::Enabled;
        HitboxMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        LastEnemyHitTime = FPlatformTime::Seconds();
    } else if (!ShouldBeEnabled && HitboxState == ESliceAndDiceHitboxState::Enabled) {
        HitboxState = ESliceAndDiceHitboxState::Disabled;
        HitboxMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void ASliceAndDice::HandleHitDetection() {
    if (HitboxState != ESliceAndDiceHitboxState::Enabled) { return; }

    const double CurrentTime = FPlatformTime::Seconds();
    const double ScaledHitDelay = CalculateHitDelay();

    if (CurrentTime <= LastEnemyHitTime + ScaledHitDelay) { return; }

    if (CurrentState == EDefensiveStructureState::Attacking) {
        LastEnemyHitTime += ScaledHitDelay;
        ProcessHitboxOverlaps();
    }
}

float ASliceAndDice::CalculateHitDelay() const {
    float BladeSpeedRatio = FMath::Clamp(CurrentBladeRotationVelocity / BladeRotationMaxSpeed,
                                         0.0f,
                                         1.0f);
    BladeSpeedRatio = FMath::Clamp(
        (BladeSpeedRatio - HitboxEnabledThreshold) / (1.0f - HitboxEnabledThreshold),
        0.0f,
        1.0f);
    return FMath::Lerp(MaxEnemyHitDelay, MinEnemyHitDelay, BladeSpeedRatio);
}

void ASliceAndDice::ProcessHitboxOverlaps() {
    TArray<AActor*> OverlappingActors;
    HitboxMesh->GetOverlappingActors(OverlappingActors);

    for (const auto& Actor : OverlappingActors) {
        if (Actor == this) { continue; }
        UDamageUtils::ApplyDamage(Actor, this, 1.0f, 200.0f, EDDDamageType::Melee);
    }
}


void ASliceAndDice::ValidateConfiguration() const {
    // Call parent validation first
    Super::ValidateConfiguration();

    // Validate components
    ensureAlways(HitboxMesh->GetStaticMesh() != nullptr);

    // Validate blade configuration values are positive
    ensureAlways(BladeRotationAcceleration > 0.0f);
    ensureAlways(BladeRotationMaxSpeed > 0.0f);
    ensureAlways(BladeRadius > 0.0f);

    // Validate hit timing configuration
    ensureAlways(MinEnemyHitDelay > 0.0f);
    ensureAlways(MaxEnemyHitDelay > MinEnemyHitDelay);
    ensureAlways(HitboxEnabledThreshold >= 0.0f && HitboxEnabledThreshold <= 1.0f);
}
