#pragma once

#include "CoreMinimal.h"
#include "BowlingBallTurret.h"
#include "Core/DependencyInjectable.h"
#include "Core/ConfigurationValidatable.h"
#include "GameFramework/Actor.h"
#include "BowlingBallTurretAmmo.generated.h"

class UProjectileMovementComponent;

/**
 * Bowling ball projectile ammunition for BowlingBallTurret defensive structures.
 * Provides heavy projectile physics with collision damage and owner filtering.
 * Integrates with entity system for faction-based damage application.
 */
UCLASS()
class DDKNOCKOFF_API ABowlingBallTurretAmmo
    : public AActor, public IEntity, public IDependencyInjectable,
      public IConfigurationValidatable {
    GENERATED_BODY()

public:
    ABowlingBallTurretAmmo();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Collision handling

    /**
     * Handle physical hit events for damage application.
     * @param HitComponent - Component that was hit
     * @param OtherActor - Actor that collided with this projectile
     * @param OtherComp - Component of the other actor
     * @param NormalImpulse - Impact normal impulse
     * @param Hit - Hit result information
     */
    UFUNCTION()
    void OnPhysicalHit(UPrimitiveComponent* HitComponent,
                       AActor* OtherActor,
                       UPrimitiveComponent* OtherComp,
                       FVector NormalImpulse,
                       const FHitResult& Hit);

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;
    void Fire();

    /**
     * Disable collision with the turret that spawned this projectile.
     * @param own - Owner turret to ignore collisions with
     */
    static void DisableCollisionWithOwner(ABowlingBallTurret* own);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    float ProjectileSpeed = 1000.0f;

    virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

    virtual EFaction GetFaction() const override;
    virtual UEntityData* GetEntityData() const override;
    virtual EEntityType GetEntityType() const override;
    virtual AActor* GetActor() override;

    // IDependencyInjectable interface
    virtual bool HasRequiredDependencies() const override;
    virtual void CollectDependencies() override;
    // End IDependencyInjectable interface

    // IConfigurationValidatable interface
    virtual void ValidateConfiguration() const override;
    // End IConfigurationValidatable interface

protected:
    // Entity data
    UPROPERTY(Transient, Instanced)
    TObjectPtr<UEntityData> EntityData;

    // Dependency injection
    UPROPERTY(Transient)
    UEntityManager* EntityManager;
};
