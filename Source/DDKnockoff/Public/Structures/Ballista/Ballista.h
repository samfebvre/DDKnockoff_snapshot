#pragma once

#include "CoreMinimal.h"
#include "Structures/DefensiveStructure.h"
#include "Ballista.generated.h"

class IBallistaAnimationInterface;
class UBallistaAnimInstance;
class ABallistaAmmo;
class UEnemyDetectionComponent;

/**
 * Large projectile-firing defensive structure with enemy tracking and aiming.
 * Features automatic target acquisition, rotation animation, and projectile spawning.
 * Uses animation notifies to synchronize firing with visual effects.
 */
UCLASS()
class DDKNOCKOFF_API ABallista : public ADefensiveStructure {
    GENERATED_BODY()

public:
    ABallista();

    // Actor lifecycle
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Combat system

    /**
     * Perform attack behavior - triggers aiming animation sequence.
     */
    virtual void Attack() override;

    /**
     * Handle animation notify to spawn and fire projectile.
     * @param Animation - Animation sequence that triggered the notify
     */
    virtual void OnFireNotifyReceived(UAnimSequenceBase* Animation) override;

    // State queries

    /**
     * Get the current target actor being tracked.
     * @return Current target or null if none
     */
    AActor* GetTargetActor() const;

    /**
     * Get the enemy detection component for this ballista.
     * @return Enemy detection component
     */
    UEnemyDetectionComponent* GetEnemyDetectionComponent() const;

    // IDependencyInjectable Interface Implementation
    virtual bool HasRequiredDependencies() const override;
    virtual void CollectDependencies() override;

    // IConfigurationValidatable Interface Implementation
    virtual void ValidateConfiguration() const override;

protected:
    // Combat logic

    /**
     * Spawn and fire a ballista projectile at the current target.
     */
    void SpawnAmmo();

    /**
     * Check if the ballista should initiate an attack sequence.
     * @return true if should attack
     */
    bool ShouldAttack() const;

    /**
     * Check if the ballista can currently attack (cooldown and target).
     * @return true if can attack
     */
    bool CanAttack() const;

    // State management

    /**
     * Update target tracking and acquisition.
     */
    void UpdateTargeting();

    /**
     * Update animation state based on current target and behavior.
     */
    void UpdateAnimationState() const;

    // Components

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UEnemyDetectionComponent> EnemyDetectionComponent;

    // Configuration

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
    TSubclassOf<ABallistaAmmo> BallistaAmmoClass;

    // Runtime state

    UPROPERTY(Transient)
    TObjectPtr<AActor> TargetActor;

    UPROPERTY(Transient)
    TObjectPtr<UBallistaAnimInstance> AnimInstance;

    UPROPERTY(Transient)
    TObjectPtr<ABallistaAmmo> BallistaAmmoInstance;
};
