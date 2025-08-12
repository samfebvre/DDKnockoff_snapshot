#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacterEnums.h"
#include "Entities/Entity.h"
#include "Core/DependencyInjectable.h"
#include "Core/ConfigurationValidatable.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "DDAICharacter.generated.h"

class UHealthComponent;
class USphereComponent;
class UBoxComponent;
class UAIAnimInstance;
class UWidgetComponent;
class UHealthBarWidget;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UHealthBarWidgetComponent;
class UCurrencySpawner;
struct FInputActionValue;

/**
 * AI-controlled enemy character with combat, detection, and currency reward systems.
 * Implements full entity interface for damage handling and faction-based interactions.
 * Features automatic targeting, attack behaviors, and death rewards.
 */
UCLASS(config=Game)
class DDKNOCKOFF_API ADDAICharacter
    : public ACharacter, public IEntity, public IDependencyInjectable,
      public IConfigurationValidatable {
    GENERATED_BODY()

public:
    ADDAICharacter();

    // Actor lifecycle
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

    // Combat system

    /**
     * Initiate an attack against the specified target.
     * @param Target - Actor to attack
     */
    void Attack(const AActor& Target);

    /**
     * Check if this character can currently perform an attack.
     * @return true if attack is possible
     */
    bool CanAttack() const;

    /**
     * Attack the target if currently able to do so.
     * @param Target - Actor to attack if possible
     */
    void AttackIfAble(AActor& Target);

    /**
     * Force character into hit reaction animation state.
     */
    void EnterHitReaction() const;

    // State queries

    /**
     * Get the closest overlapping defensive structure for pathfinding.
     * @return Weak pointer to closest structure or null
     */
    TWeakObjectPtr<AActor> GetClosestOverlappingStructure() const;

    EEnemyPoseState GetCurrentPoseState() const;
    CharacterActorOverlapState GetActorOverlapState() const;

    // Event handlers

    UFUNCTION()
    void OnHitboxOverlap(UPrimitiveComponent* OverlappedComponent,
                         AActor* OtherActor,
                         UPrimitiveComponent* OtherComp,
                         int32 OtherBodyIndex,
                         bool bFromSweep,
                         const FHitResult& SweepResult);

    UFUNCTION()
    void OnDeath();

    // Events

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCapsuleColliderHit,
                                                UPrimitiveComponent*,
                                                OtherComp);

    UPROPERTY(BlueprintAssignable)
    FOnCapsuleColliderHit Evt_OnCapsuleColliderHit;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTookKnockback);

    UPROPERTY(BlueprintAssignable)
    FOnTookKnockback Evt_OnTookKnockback;

    // IEntity Interface Implementation
    virtual void TakeDamage(const FDamagePayload& DamagePayload) override;
    virtual void OnHitboxBeginNotifyReceived(UAnimSequenceBase* Animation) override;
    virtual void OnHitboxEndNotifyReceived(UAnimSequenceBase* Animation) override;
    virtual void TakeKnockback(const FVector& Direction, const float Strength) override;
    virtual EFaction GetFaction() const override;
    virtual AActor* GetActor() override;
    virtual float GetHalfHeight() const override;
    virtual UEntityData* GetEntityData() const override { return EntityData; }
    virtual EEntityType GetEntityType() const override { return EEntityType::Character; }

    // IDependencyInjectable Interface Implementation
    virtual bool HasRequiredDependencies() const override;
    virtual void CollectDependencies() override;

    // IConfigurationValidatable Interface Implementation
    virtual void ValidateConfiguration() const override;

protected:
    // Utility functions

    UFUNCTION(BlueprintCallable, Category = "Character")
    FVector GetCapsuleBottomLocation() const;

    // Core components

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UHealthComponent> HealthComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    TObjectPtr<UBoxComponent> HitboxComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCurrencySpawner> CurrencySpawner;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCapsuleComponent> TargetDetectionCollider;

    UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UAIAnimInstance> AnimInstance;

    // Configuration

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
    int32 CurrencyRewardAmount = 10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
    int32 MinimumCrystalCount = 1;

    // Runtime state

    FVector TargetLocation;
    CharacterActorOverlapState ActorOverlapState = CharacterActorOverlapState::None;

    UPROPERTY(Transient)
    TArray<AActor*> ActorsInDetectionRange;

    UPROPERTY(Transient, Instanced)
    TObjectPtr<UEntityData> EntityData;

    UPROPERTY(Transient)
    TWeakObjectPtr<AActor> ClosestOverlappingStructure;

private:
    // Dependencies

    UPROPERTY(Transient)
    UEntityManager* EntityManager;
};
