#include "Structures/BowlingBallTurret/BowlingBallTurretAnimInstance.h"

#include "Structures/BowlingBallTurret/BowlingBallTurret.h"
#include "Utils/MathUtils.h"
#include "Utils/GeometryUtils.h"

void UBowlingBallTurretAnimInstance::PointToTargetLocation(const FVector& TargetLocation) {
    // Cast owner to BowlingBallTurret
    const ABowlingBallTurret* BowlingBallTurret = Cast<ABowlingBallTurret>(GetOwningActor());
    // get BowlingBallTurret skeletal mesh
    const USkeletalMeshComponent* BowlingBallTurretMesh = BowlingBallTurret->GetSkeletonMesh();
    // get head bone position
    // TODO - cache this at some point - when the BowlingBallTurret is placed, and in begin play
    const FVector HeadBonePosition = BowlingBallTurretMesh->GetBoneLocation(
        FName("head"),
        EBoneSpaces::WorldSpace);

    // Use GeometryUtils to calculate targeting data
    const FTurretTargetingData TargetingData = UGeometryUtils::CalculateTurretTargeting(
        HeadBonePosition,
        TargetLocation);

    if (!TargetingData.bValidTarget) { return; }

    // Apply relative rotation to the desired body rotation
    FRotator DesiredBodyRotation = TargetingData.BodyRotation;
    DesiredBodyRotation -= BowlingBallTurret->GetActorRotation();
    DesiredBodyRotation.Roll = 0.0f;
    DesiredBodyRotation.Pitch = 0.0f;

    BodyRotation = MathUtils::SmoothDampRotator(
        BodyRotation,
        DesiredBodyRotation,
        CurrentBodyRotationVelocity,
        RotationSmoothTime,
        GetWorld()->GetDeltaSeconds(),
        RotationMaxSpeed);

    // Apply elevation angle with proper sign handling
    float ElevationAngle = TargetingData.ElevationAngle;
    if (TargetLocation.Z > HeadBonePosition.Z) { ElevationAngle = -ElevationAngle; }

    const FRotator DesiredHeadRotation = FRotator(0, 0.0f, ElevationAngle);
    HeadRotation = MathUtils::SmoothDampRotator(
        HeadRotation,
        DesiredHeadRotation,
        CurrentHeadRotationVelocity,
        RotationSmoothTime,
        GetWorld()->GetDeltaSeconds(),
        RotationMaxSpeed);
}

void UBowlingBallTurretAnimInstance::PlayAttackAnim() {
    // Play the attack montage
    Montage_Play(AttackMontage, 1.0f);
    // Set the animation state to attacking
    CurrentBowlingBallTurretAnimationState = EBowlingBallTurretAnimationState::Attacking;
}

void UBowlingBallTurretAnimInstance::OnMontageBlendingOutStarted(
    UAnimMontage* Montage,
    bool bInterrupted) {
    if (CurrentBowlingBallTurretAnimationState == EBowlingBallTurretAnimationState::Attacking) {
        CurrentBowlingBallTurretAnimationState = EBowlingBallTurretAnimationState::Idle;
    }
}

void UBowlingBallTurretAnimInstance::NativeBeginPlay() {
    Super::NativeBeginPlay();

    // Bind to the montage ended event
    OnMontageBlendingOut.RemoveDynamic(this,
                                       &UBowlingBallTurretAnimInstance::
                                       OnMontageBlendingOutStarted);
    OnMontageBlendingOut.AddDynamic(this,
                                    &UBowlingBallTurretAnimInstance::OnMontageBlendingOutStarted);
}
