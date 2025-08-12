#include "Player/PlayerAnimInstance.h"

void UPlayerAnimInstance::SetPoseState(const EPlayerPoseState NewPoseState) {
    CurrentPoseState = NewPoseState;
}

void UPlayerAnimInstance::SetAttackState(const EPlayerAttackState NewAttackState) {
    CurrentAttackState = NewAttackState;
}

bool UPlayerAnimInstance::CanAttack() const {
    return CurrentPoseState == EPlayerPoseState::Locomotion || CurrentAttackState ==
           EPlayerAttackState::Attacking_PostComboTrigger;
}

void UPlayerAnimInstance::Attack() {
    if (ComboAttackMontages.Num() == 0) { return; }

    SetPoseState(EPlayerPoseState::Attack);
    SetAttackState(EPlayerAttackState::Attacking_PreComboWindowBegin);

    // Get the current combo montage
    UAnimMontage* ComboMontage = ComboAttackMontages[CurrentComboIndex];
    if (!ComboMontage) { return; }

    // Play the combo montage (this will interrupt any currently playing montage)
    LastPlayedAttackMontage = ComboMontage;
    Montage_Play(ComboMontage, AttackSpeed);

    // Configure blend times for smooth transitions
    // ComboMontage->BlendOut.SetBlendTime(0.15f / AttackSpeed);
    // ComboMontage->BlendIn.SetBlendTime(0.1f / AttackSpeed);

    // Advance to next combo with wraparound
    CurrentComboIndex = (CurrentComboIndex + 1) % ComboAttackMontages.Num();
}

void UPlayerAnimInstance::ResetCombo() { CurrentComboIndex = 0; }

void UPlayerAnimInstance::EnterHitReaction() {
    CleanUpLastAttackMontage();

    // Reset combo when hit
    ResetCombo();

    HitReactionAnimTrigger = true;

    SetPoseState(EPlayerPoseState::HitReaction);
    SetAttackState(EPlayerAttackState::None);

    // Clear any existing timer and start new one
    GetWorld()->GetTimerManager().ClearTimer(HitReactionTimerHandle);
    GetWorld()->GetTimerManager().SetTimer(HitReactionTimerHandle,
                                           this,
                                           &UPlayerAnimInstance::RecoverFromHitReaction,
                                           HitReactionDuration,
                                           false);
}

void UPlayerAnimInstance::RecoverFromHitReaction() {
    if (CurrentPoseState == EPlayerPoseState::HitReaction) {
        SetPoseState(EPlayerPoseState::Locomotion);
        HitReactionAnimTrigger = false;
    }
}

void UPlayerAnimInstance::OnMontageBlendingOutStarted(UAnimMontage* Montage, bool bInterrupted) {
    if (CurrentPoseState == EPlayerPoseState::Attack) {
        // Reset combo when attack animation ends naturally (not when interrupted by another combo)
        if (bInterrupted == false) {
            SetPoseState(EPlayerPoseState::Locomotion);
            ResetCombo();
            SetAttackState(EPlayerAttackState::None);
        }
    }
}

void UPlayerAnimInstance::NativeBeginPlay() {
    Super::NativeBeginPlay();

    RootMotionMode = ERootMotionMode::IgnoreRootMotion;
    ValidateConfiguration();

    // Bind to the montage ended event
    OnMontageBlendingOut.RemoveDynamic(this, &UPlayerAnimInstance::OnMontageBlendingOutStarted);
    OnMontageBlendingOut.AddDynamic(this, &UPlayerAnimInstance::OnMontageBlendingOutStarted);
}

void UPlayerAnimInstance::CleanUpLastAttackMontage() {
    if (LastPlayedAttackMontage == nullptr) { return; }

    if (Montage_IsPlaying(LastPlayedAttackMontage)) { Montage_Stop(0.0f, LastPlayedAttackMontage); }

    LastPlayedAttackMontage = nullptr;
}

// IConfigurationValidatable interface implementation
void UPlayerAnimInstance::ValidateConfiguration() const {
    // Validate Animation Properties
    ensureAlways(HitReactionDuration > 0.0f);
    ensureAlways(AttackSpeed > 0.0f);

    // Validate Combo System Properties
    ensureAlwaysMsgf(ComboAttackMontages.Num() > 0,
                     TEXT("ComboAttackMontages array must contain at least one montage"));

    for (int32 i = 0; i < ComboAttackMontages.Num(); i++) {
        ensureAlwaysMsgf(ComboAttackMontages[i] != nullptr,
                         TEXT("ComboAttackMontages[%d] is null"),
                         i);
    }
}
