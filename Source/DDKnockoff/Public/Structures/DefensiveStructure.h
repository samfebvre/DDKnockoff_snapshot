#pragma once

#include "CoreMinimal.h"
#include "StructurePlacementEnums.h"
#include "Entities/Entity.h"
#include "Core/DependencyInjectable.h"
#include "Core/ConfigurationValidatable.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "DefensiveStructure.generated.h"

class UHealthComponent;
class UNavArea;
class UNavModifierComponent;
class UAnimInstance;
class UDefensiveStructureNavArea;
class UStructurePreviewComponent;

/**
 * State enumeration for all defensive structure behaviors.
 */
UENUM(BlueprintType)
enum class EDefensiveStructureState : uint8 {
    Idle UMETA(DisplayName = "Idle"),
    // Structure is active but not engaging targets
    Attacking UMETA(DisplayName = "Attacking"),
    // Structure is currently attacking a target
    MAX UMETA(Hidden)
};

/**
 * Base class for all defensive structures in the game.
 * Provides placement preview system, navigation modification, combat capabilities, and death handling.
 * Implements full entity interface for damage processing and faction-based interactions.
 */
UCLASS()
class DDKNOCKOFF_API ADefensiveStructure
    : public AActor, public IEntity, public IDependencyInjectable,
      public IConfigurationValidatable {
    GENERATED_BODY()

public:
    ADefensiveStructure();

    // Actor lifecycle
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

    // Placement system

    /**
     * Initialize structure for placement preview mode.
     */
    void OnStartedPreviewing();

    /**
     * Finalize structure placement and activate gameplay systems.
     */
    void OnPlaced();

    /**
     * Update preview material color based on placement validity.
     * @param ValidityState - Current placement validity
     * @param InvalidityReason - Specific reason for invalidity
     */
    void UpdatePreviewMaterialColor(EStructurePlacementValidityState ValidityState,
                                    EStructurePlacementInvalidityReason InvalidityReason) const;

    /**
     * Get the collision mesh used for placement preview validation.
     * @return Preview collision mesh component
     */
    UStaticMeshComponent* GetPreviewCollisionMesh() const;

    // Combat system

    /**
     * Check if structure can perform an attack (cooldown expired).
     * @return true if attack is off cooldown
     */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsAttackOffCooldown() const;

    /**
     * Record that an attack has been performed for cooldown tracking.
     */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RecordAttack();

    // State queries

    EDefensiveStructureState GetCurrentState() const;
    TObjectPtr<UAnimInstance> GetBaseAnimInstance() const { return BaseAnimInstance; }
    USkeletalMeshComponent* GetSkeletonMesh() const;

    /**
     * Get the currency cost to place this structure.
     * @return Currency cost
     */
    int GetCurrencyCostToPlaceStructure() const { return CurrencyCostToPlaceStructure; }

    // Event handlers

    UFUNCTION()
    void OnDeath();

    // IEntity Interface Implementation
    virtual void TakeDamage(const FDamagePayload& DamagePayload) override;
    virtual void OnHitboxBeginNotifyReceived(UAnimSequenceBase* Animation) override {}
    virtual void OnHitboxEndNotifyReceived(UAnimSequenceBase* Animation) override {}
    virtual EFaction GetFaction() const override;
    virtual AActor* GetActor() override;
    virtual UEntityData* GetEntityData() const override;
    virtual EEntityType GetEntityType() const override;
    virtual bool IsCurrentlyTargetable() const override;

    // IDependencyInjectable Interface Implementation
    virtual bool HasRequiredDependencies() const override;
    virtual void CollectDependencies() override;

    // IConfigurationValidatable Interface Implementation
    virtual void ValidateConfiguration() const override;

protected:
    /**
     * Virtual attack method for derived structures to implement.
     * Called when structure should perform its attack behavior.
     */
    virtual void Attack() {}

    // Core components

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* SkeletonMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* PhysicalCollisionMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* HurtboxMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* PreviewCollisionMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UHealthComponent> HealthComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UNavModifierComponent> NavModifierComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStructurePreviewComponent> StructurePreviewComponent;

    // Configuration

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Structure")
    int CurrencyCostToPlaceStructure = 10;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    TSubclassOf<UNavArea> NavAreaClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float AttackCooldownSeconds = 1.0f;

    // Runtime state

    EStructurePlacementState StructurePlacementState = EStructurePlacementState::NotPreviewing;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "State")
    EDefensiveStructureState CurrentState = EDefensiveStructureState::Idle;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "Combat")
    float LastAttackTime = 0.0f;

    UPROPERTY(Transient)
    TObjectPtr<UAnimInstance> BaseAnimInstance;

    UPROPERTY(Transient, Instanced)
    TObjectPtr<UEntityData> EntityData;

private:
    // Dependencies

    UPROPERTY(Transient)
    UEntityManager* EntityManager;
};
