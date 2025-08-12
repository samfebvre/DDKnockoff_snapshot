#pragma once

#include "CoreMinimal.h"
#include "Entities/Entity.h"
#include "Core/DependencyInjectable.h"
#include "Core/ConfigurationValidatable.h"
#include "GameFramework/Actor.h"
#include "ResourceChest.generated.h"

class UNavArea_Obstacle;
class UCurrencySpawner;
class UBoxComponent;
class UNavModifierComponent;
class UResourceChestAnimInstance;

/**
 * Chest state enumeration for interaction progression.
 */
UENUM(BlueprintType)
enum class EChestState : uint8 {
    Closed,
    // Chest is closed and ready for interaction
    Opened,
    // Chest has been opened and rewarded
    MAX UMETA(Hidden)
};

/**
 * Chest growth state for scaling animations.
 */
UENUM(BlueprintType)
enum class EChestGrowState : uint8 {
    FullSize,
    // Chest at normal size
    Shrinking,
    // Chest is scaling down
    Growing,
    // Chest is scaling up
    FullyShrunk,
    // Chest is fully scaled down
    MAX UMETA(Hidden)
};

/**
 * Minimum crystal spawning options for configurable reward systems.
 */
UENUM(BlueprintType)
enum class EMinimumCrystalOption : uint8 {
    Unenforced,
    // No minimum required
    SpecificMinimum,
    // Use a specific minimum value
    RandomRange,
    // Random minimum between two values
    MAX UMETA(Hidden)
};

/**
 * Interactive resource chest that spawns currency when opened.
 * Provides animated chest with configurable rewards and scaling effects.
 * Integrates with entity system for interaction and navigation.
 */
UCLASS()
class DDKNOCKOFF_API AResourceChest
    : public AActor, public IEntity, public IDependencyInjectable,
      public IConfigurationValidatable {
    GENERATED_BODY()

public:
    AResourceChest();

    // Actor lifecycle
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

    /**
     * Reset chest to closed state for reuse.
     */
    void ResetChest();

    // IEntity Interface
    virtual AActor* GetActor() override;
    virtual UEntityData* GetEntityData() const override;
    virtual EEntityType GetEntityType() const override;
    virtual EFaction GetFaction() const override;
    virtual void Interact() override;

    // IDependencyInjectable Interface
    virtual bool HasRequiredDependencies() const override;
    virtual void CollectDependencies() override;

    // IConfigurationValidatable Interface
    virtual void ValidateConfiguration() const override;

protected:
    // Components

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* SkeletonMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* PhysicalCollisionMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* InteractCollisionBox;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UNavModifierComponent> NavModifierComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    TSubclassOf<UNavArea_Obstacle> NavAreaClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCurrencySpawner> CurrencySpawner;

    // Animation system

    UPROPERTY(Transient)
    TObjectPtr<UResourceChestAnimInstance> AnimInstance;

    // State management helpers

    void SetChestState(EChestState NewState);
    void SetChestGrowState(EChestGrowState NewState);
    void StartShrinking();
    void UpdateScale(float DeltaTime);
    void DisableChest() const;
    void EnableChest() const;

    // Chest configuration

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest")
    EChestState ChestState = EChestState::Closed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest")
    EChestGrowState ChestGrowState = EChestGrowState::FullSize;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest")
    float ShrinkDelay = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest")
    float ScaleChangeDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest")
    int32 RewardedCurrencyAmount = 100;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chest")
    EMinimumCrystalOption MinimumCrystalOption = EMinimumCrystalOption::Unenforced;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Chest",
        meta = (EditCondition = "MinimumCrystalOption == EMinimumCrystalOption::SpecificMinimum"),
        meta =(EditConditionHides = true))
    int32 SpecificMinimumCrystalCount = 4;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Chest",
        meta = (EditCondition = "MinimumCrystalOption == EMinimumCrystalOption::RandomRange"),
        meta =(EditConditionHides = true))
    int32 MinRandomCrystalCount = 2;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Chest",
        meta = (EditCondition = "MinimumCrystalOption == EMinimumCrystalOption::RandomRange"),
        meta =(EditConditionHides = true))
    int32 MaxRandomCrystalCount = 6;

    // Runtime state

    FTimerHandle ShrinkTimerHandle;
    float ScaleProgress = 1.0f;

    // Entity data

    UPROPERTY(Transient, Instanced)
    TObjectPtr<UEntityData> EntityData;

    // Dependencies

    UPROPERTY(Transient)
    UEntityManager* EntityManager;
};
