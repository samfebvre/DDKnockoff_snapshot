#pragma once

#include "CoreMinimal.h"
#include "Ballista.h"
#include "Core/DependencyInjectable.h"
#include "Core/ConfigurationValidatable.h"
#include "GameFramework/Actor.h"
#include "BallistaAmmo.generated.h"

class UProjectileMovementComponent;

/**
 * Projectile ammunition for Ballista defensive structures.
 * Handles projectile physics, collision detection, and damage application.
 * Integrates with entity system for faction-based damage and tracking.
 */
UCLASS()
class DDKNOCKOFF_API ABallistaAmmo
    : public AActor, public IEntity, public IDependencyInjectable,
      public IConfigurationValidatable {
    GENERATED_BODY()

public:
    ABallistaAmmo();

    // Actor lifecycle
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

    // Projectile system

    /**
     * Launch the projectile with configured speed and trajectory.
     */
    void Fire();

    // Collision event handlers

    /**
     * Handle physical collision with world objects.
     * @param HitComponent - Component that was hit
     * @param OtherActor - Actor that was hit
     * @param OtherComp - Other component involved in collision
     * @param NormalImpulse - Impact force normal
     * @param Hit - Hit result data
     */
    UFUNCTION()
    void OnPhysicalHit(UPrimitiveComponent* HitComponent,
                       AActor* OtherActor,
                       UPrimitiveComponent* OtherComp,
                       FVector NormalImpulse,
                       const FHitResult& Hit);

    /**
     * Handle hitbox overlap for damage detection.
     * @param OverlappedComponent - Component that overlapped
     * @param OtherActor - Actor that was overlapped
     * @param OtherComp - Other component involved in overlap
     * @param OtherBodyIndex - Body index of other component
     * @param bFromSweep - Whether overlap was from sweep
     * @param SweepResult - Sweep result data
     */
    UFUNCTION()
    void OnHitboxOverlap(UPrimitiveComponent* OverlappedComponent,
                         AActor* OtherActor,
                         UPrimitiveComponent* OtherComp,
                         int32 OtherBodyIndex,
                         bool bFromSweep,
                         const FHitResult& SweepResult);

    // IEntity Interface
    virtual EFaction GetFaction() const override;
    virtual UEntityData* GetEntityData() const override;
    virtual EEntityType GetEntityType() const override;
    virtual AActor* GetActor() override;

    // IDependencyInjectable Interface
    virtual bool HasRequiredDependencies() const override;
    virtual void CollectDependencies() override;

    // IConfigurationValidatable Interface
    virtual void ValidateConfiguration() const override;

protected:
    // Components

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> VisualMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> HitboxMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

    // Configuration

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    float ProjectileSpeed = 1000.0f;

    // Entity data

    UPROPERTY(Transient, Instanced)
    TObjectPtr<UEntityData> EntityData;

    // Dependencies

    UPROPERTY(Transient)
    UEntityManager* EntityManager;
};
