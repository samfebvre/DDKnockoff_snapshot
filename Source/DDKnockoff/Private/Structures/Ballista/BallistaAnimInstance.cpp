#include "Structures/Ballista/BallistaAnimInstance.h"

#include "Structures/Ballista/Ballista.h"
#include "Utils/MathUtils.h"
#include "Utils/GeometryUtils.h"

void UBallistaAnimInstance::PointToTargetLocation(const FVector& TargetLocation) {
    // Cast owner to ballista
    const ABallista* Ballista = Cast<ABallista>(GetOwningActor());
    // get ballista skeletal mesh
    const USkeletalMeshComponent* BallistaMesh = Ballista->GetSkeletonMesh();
    // get head bone position
    // TODO - cache this at some point - when the ballista is placed, and in begin play
    const FVector HeadBonePosition = BallistaMesh->GetBoneLocation(
        FName("head"),
        EBoneSpaces::WorldSpace);

    // Use GeometryUtils to calculate targeting data
    const FTurretTargetingData TargetingData = UGeometryUtils::CalculateTurretTargeting(
        HeadBonePosition,
        TargetLocation);

    if (!TargetingData.bValidTarget) { return; }

    // Apply relative rotation to the desired body rotation
    FRotator DesiredBodyRotation = TargetingData.BodyRotation;
    DesiredBodyRotation -= Ballista->GetActorRotation();
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

void UBallistaAnimInstance::PlayAttackAnim() {
    // Play the attack montage
    Montage_Play(AttackMontage, 1.0f);
    // Set the animation state to attacking
    CurrentBallistaAnimationState = EBallistaAnimationState::Attacking;
}

void UBallistaAnimInstance::OnMontageBlendingOutStarted(UAnimMontage* Montage, bool bInterrupted) {
    if (CurrentBallistaAnimationState == EBallistaAnimationState::Attacking) {
        CurrentBallistaAnimationState = EBallistaAnimationState::Idle;
    }
}

void UBallistaAnimInstance::NativeBeginPlay() {
    Super::NativeBeginPlay();

    // Bind to the montage ended event
    OnMontageBlendingOut.RemoveDynamic(this, &UBallistaAnimInstance::OnMontageBlendingOutStarted);
    OnMontageBlendingOut.AddDynamic(this, &UBallistaAnimInstance::OnMontageBlendingOutStarted);
}
