#pragma once

#include "CoreMinimal.h"
#include "Entities/Entity.h"
#include "Core/DependencyInjectable.h"
#include "Core/ConfigurationValidatable.h"
#include "GameFramework/Actor.h"
#include "CrystalStructure.generated.h"

class UNavModifierComponent;
class UCrystalAnimInstance;
class UBoxComponent;
class UAnimInstance;

/**
 * Crystal structure entity with health, animation, and navigation systems.
 * Provides animated crystal objects that can be damaged and destroyed.
 * Integrates with entity system for faction management and navigation.
 */
UCLASS()
class DDKNOCKOFF_API ACrystalStructure
    : public AActor, public IEntity, public IDependencyInjectable,
      public IConfigurationValidatable {
    GENERATED_BODY()

public:
    ACrystalStructure();

    // Actor lifecycle
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

    // Combat system
    virtual void TakeDamage(const FDamagePayload& DamagePayload) override;

    /**
     * Handle crystal destruction and cleanup.
     */
    UFUNCTION()
    void OnDeath();

    // IEntity Interface
    virtual EFaction GetFaction() const override;
    virtual AActor* GetActor() override;
    virtual UEntityData* GetEntityData() const override;
    virtual EEntityType GetEntityType() const override;

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
    UStaticMeshComponent* HurtboxMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<class UHealthComponent> HealthComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UNavModifierComponent> NavModifierComponent;

    // Animation system

    UPROPERTY(Transient)
    TObjectPtr<UCrystalAnimInstance> AnimInstance;

    // Entity data

    // TODO - entity data should be instanced by default, might not be necessary to specify it here or anywhere else that we are.
    UPROPERTY(Transient, Instanced)
    TObjectPtr<UEntityData> EntityData;

    // Dependencies

    UPROPERTY(Transient)
    UEntityManager* EntityManager;
};
