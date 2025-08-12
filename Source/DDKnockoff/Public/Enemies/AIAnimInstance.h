#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacterEnums.h"
#include "Animation/AnimInstance.h"
#include "AIAnimInstance.generated.h"

/**
 * Animation instance for AI enemy characters.
 * Handles pose state management, attack animations, and hit reactions.
 * Coordinates between AI behavior and animation system for responsive enemy movement.
 */
UCLASS()
class DDKNOCKOFF_API UAIAnimInstance : public UAnimInstance {
    GENERATED_BODY()

public:
    // UAnimInstance Interface
    virtual void NativeBeginPlay() override;

    // State management

    /**
     * Set the current pose state for the enemy.
     * @param NewPoseState - New pose state to transition to
     */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetPoseState(EEnemyPoseState NewPoseState);

    /**
     * Check if the enemy can currently perform an attack.
     * @return true if attack animations can be played
     */
    bool CanAttack() const;

    /**
     * Get the current pose state.
     * @return Current pose state
     */
    EEnemyPoseState GetCurrentPoseState() const { return CurrentPoseState; }

    // Attack system

    /**
     * Trigger an attack animation sequence.
     */
    void Attack();

    /**
     * Play a random attack montage from the available set.
     */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayRandomAttackMontage();

    /**
     * Check if a sequence is part of the attack animation set.
     * @param AnimSequence - Animation sequence to check
     * @return true if sequence is in attack montages array
     */
    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool IsSequenceInAttackAnimMontages(const UAnimSequenceBase* AnimSequence) const;

    // Hit reaction system

    /**
     * Enter hit reaction state with timed recovery.
     */
    void EnterHitReaction();

    /**
     * Recover from hit reaction state.
     */
    void RecoverFromHitReaction();

protected:
    // Montage event handling

    /**
     * Handle montage blending out events.
     * @param Montage - Montage that is blending out
     * @param bInterrupted - Whether the montage was interrupted
     */
    UFUNCTION()
    void OnMontageBlendingOutStarted(UAnimMontage* Montage, bool bInterrupted);

    /**
     * Clean up resources from the last played attack montage.
     */
    void CleanUpLastAttackMontage();

    // Configuration

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    TArray<TObjectPtr<UAnimMontage>> AttackAnimMontages;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    float HitReactionDuration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "State")
    float AttackSpeed = 1.0f;

    // Runtime state

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    EEnemyPoseState CurrentPoseState = EEnemyPoseState::Locomotion;

    UPROPERTY(Transient)
    UAnimMontage* LastPlayedAttackMontage = nullptr;

    // TODO - This should be handled more intelligently, it shouldn't just be an exposed variable.
    UPROPERTY(BlueprintReadWrite, Category= "Animation")
    bool HitReactionAnimTrigger = false;

    // Timers

    FTimerHandle HitReactionTimerHandle;
};
