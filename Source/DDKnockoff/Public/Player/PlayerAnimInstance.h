#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Core/ConfigurationValidatable.h"
#include "PlayerAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EPlayerPoseState : uint8 {
    Locomotion UMETA(DisplayName = "Locomotion"),
    Attack UMETA(DisplayName = "Attack"),
    HitReaction UMETA(DisplayName = "HitReaction"),

    MAX UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EPlayerAttackState : uint8 {
    None UMETA(DisplayName = "Idle"),
    Attacking_PreComboWindowBegin UMETA(DisplayName = "Attacking_PreComboWindowBegin"),
    Attacking_PreComboTrigger UMETA(DisplayName = "Attacking_PreComboTrigger"),
    Attacking_PostComboTrigger UMETA(DisplayName = "Attacking_PostComboTrigger"),

    MAX UMETA(Hidden)
};

/**
 * Animation instance for the player character, providing animation state management
 * and attack functionality similar to AIAnimInstance but tailored for player needs
 */
UCLASS()
class DDKNOCKOFF_API UPlayerAnimInstance : public UAnimInstance, public IConfigurationValidatable {
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetPoseState(EPlayerPoseState NewPoseState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetAttackState(EPlayerAttackState NewPoseState);

    bool CanAttack() const;
    void Attack();
    void ResetCombo();
    void EnterHitReaction();
    void RecoverFromHitReaction();

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    EPlayerPoseState CurrentPoseState = EPlayerPoseState::Locomotion;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    EPlayerAttackState CurrentAttackState = EPlayerAttackState::None;

    // Combo attack montages - array of montages for combo system
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TArray<TObjectPtr<UAnimMontage>> ComboAttackMontages;

    // Hit reaction duration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    float HitReactionDuration = 1.0f;

    // Attack speed multiplier
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
    float AttackSpeed = 1.0f;

    // Blueprint trigger for hit reaction animation
    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool HitReactionAnimTrigger = false;

    // IConfigurationValidatable interface
    virtual void ValidateConfiguration() const override;
    // End IConfigurationValidatable interface

protected:
    virtual void NativeBeginPlay() override;

    UFUNCTION()
    void OnMontageBlendingOutStarted(UAnimMontage* Montage, bool bInterrupted);

private:
    FTimerHandle HitReactionTimerHandle;

    UPROPERTY(Transient)
    UAnimMontage* LastPlayedAttackMontage = nullptr;

    int32 CurrentComboIndex = 0;

    void CleanUpLastAttackMontage();
};
