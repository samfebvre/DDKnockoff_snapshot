#include "Enemies/AIAnimInstance.h"

void UAIAnimInstance::SetPoseState(const EEnemyPoseState NewPoseState) {
    CurrentPoseState = NewPoseState;
}

bool UAIAnimInstance::CanAttack() const { return CurrentPoseState == EEnemyPoseState::Locomotion; }

void UAIAnimInstance::EnterHitReaction() {
    // TODO - probably want to make it so that the attack stop stuff happens by exiting the attack state or something.
    CleanUpLastAttackMontage();

    HitReactionAnimTrigger = true;

    SetPoseState(EEnemyPoseState::HitReaction);

    GetWorld()->GetTimerManager().ClearTimer(HitReactionTimerHandle);
    GetWorld()->GetTimerManager().SetTimer(HitReactionTimerHandle,
                                           this,
                                           &UAIAnimInstance::RecoverFromHitReaction,
                                           HitReactionDuration,
                                           false);
};

void UAIAnimInstance::RecoverFromHitReaction() {
    if (CurrentPoseState == EEnemyPoseState::HitReaction) {
        SetPoseState(EEnemyPoseState::Locomotion);
    }
}

void UAIAnimInstance::OnMontageBlendingOutStarted(UAnimMontage* Montage, bool bInterrupted) {
    if (CurrentPoseState == EEnemyPoseState::Attack) { SetPoseState(EEnemyPoseState::Locomotion); }
}

void UAIAnimInstance::NativeBeginPlay() {
    Super::NativeBeginPlay();

    // Bind to the montage ended event
    OnMontageBlendingOut.RemoveDynamic(this, &UAIAnimInstance::OnMontageBlendingOutStarted);
    OnMontageBlendingOut.AddDynamic(this, &UAIAnimInstance::OnMontageBlendingOutStarted);
}

void UAIAnimInstance::Attack() {
    SetPoseState(EEnemyPoseState::Attack);
    // CleanUpLastAttackMontage();
    PlayRandomAttackMontage();
}

void UAIAnimInstance::PlayRandomAttackMontage() {
    if (AttackAnimMontages.Num() <= 0) { return; }

    const int32 RandomIndex = FMath::RandRange(0, AttackAnimMontages.Num() - 1);
    LastPlayedAttackMontage = AttackAnimMontages[RandomIndex];
    Montage_Play(LastPlayedAttackMontage, AttackSpeed);
    LastPlayedAttackMontage->BlendOut.SetBlendTime(2.0f / AttackSpeed);
    LastPlayedAttackMontage->BlendIn.SetBlendTime(0.3f / AttackSpeed);
}

bool UAIAnimInstance::IsSequenceInAttackAnimMontages(const UAnimSequenceBase* Sequence) const {
    return AttackAnimMontages.Contains(Sequence);
}

void UAIAnimInstance::CleanUpLastAttackMontage() {
    if (LastPlayedAttackMontage == nullptr) { return; }
    if (Montage_IsPlaying(LastPlayedAttackMontage)) { Montage_Stop(0.0f, LastPlayedAttackMontage); }
    LastPlayedAttackMontage = nullptr;
}
