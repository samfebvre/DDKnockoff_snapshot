#pragma once

#include "CoreMinimal.h"
#include "Structures/DefensiveStructure.h"
#include "BouncerBlockade.generated.h"

class UBouncerBlockadeAnimInstance;
class UEnemyDetectionComponent;
class UCapsuleComponent;

/**
 * Defensive structure with multiple hitboxes that bounces enemies away.
 * Uses animation-driven attack timing with three hitbox components.
 */
UCLASS()
class DDKNOCKOFF_API ABouncerBlockade : public ADefensiveStructure {
    GENERATED_BODY()

public:
    ABouncerBlockade();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /**
     * Get the enemy detection component for this bouncer.
     * @return Enemy detection component instance
     */
    UEnemyDetectionComponent* GetEnemyDetectionComponent();

    /**
     * Get the animation instance for this bouncer.
     * @return Animation instance
     */
    UBouncerBlockadeAnimInstance* GetAnimInstance() const { return AnimInstance; }

    // IDependencyInjectable Interface Implementation
    virtual bool HasRequiredDependencies() const override;
    virtual void CollectDependencies() override;

    // IConfigurationValidatable Interface Implementation
    virtual void ValidateConfiguration() const override;

protected:
    /**
     * Create and configure a hitbox component attached to a specific socket.
     * @param ComponentName - Name for the component
     * @param SocketName - Socket to attach to
     * @return Configured hitbox component
     */
    UCapsuleComponent* CreateHitboxComponent(const FName& ComponentName, const FName& SocketName);

    /**
     * Configure collision settings for a hitbox component.
     * @param HitboxComponent - Component to configure
     */
    static void SetupHitboxCollision(UCapsuleComponent* HitboxComponent);

    // State management
    void UpdateStructureState();
    void UpdateAttackState();
    virtual void Attack() override;

    /**
     * Enable or disable collision on all hitbox components.
     * @param bEnabled - Whether collision should be enabled
     */
    void SetHitboxCollisionEnabled(bool bEnabled) const;

    // Animation callbacks
    UFUNCTION()
    void OnAnimMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnHitboxOverlap(UPrimitiveComponent* OverlappedComponent,
                         AActor* OtherActor,
                         UPrimitiveComponent* OtherComp,
                         int32 OtherBodyIndex,
                         bool bFromSweep,
                         const FHitResult& SweepResult);

    UFUNCTION()
    virtual void OnHitboxBeginNotifyReceived(UAnimSequenceBase* animSequence) override;

    UFUNCTION()
    virtual void OnHitboxEndNotifyReceived(UAnimSequenceBase* animSequence) override;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UEnemyDetectionComponent> EnemyDetectionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCapsuleComponent> HitboxComponent1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCapsuleComponent> HitboxComponent2;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCapsuleComponent> HitboxComponent3;

    // Runtime state
    UPROPERTY(Transient)
    TObjectPtr<UBouncerBlockadeAnimInstance> AnimInstance;
};
