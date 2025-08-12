#pragma once

#include "CoreMinimal.h"
#include "Structures/DefensiveStructure.h"
#include "BowlingBallTurret.generated.h"

class ABowlingBallTurretAmmo;
class UBowlingBallTurretAnimInstance;
class UEnemyDetectionComponent;

/**
 * Defensive turret that spawns and fires bowling ball projectiles at detected enemies.
 * Features rotating animation that aims at targets before firing.
 */
UCLASS()
class DDKNOCKOFF_API ABowlingBallTurret : public ADefensiveStructure {
    GENERATED_BODY()

public:
    ABowlingBallTurret();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnFireNotifyReceived(UAnimSequenceBase* Animation) override;

    // IConfigurationValidatable Interface Implementation
    virtual void ValidateConfiguration() const override;

    // Public access for tests and external systems
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UEnemyDetectionComponent> EnemyDetectionComponent;

    UPROPERTY(Transient)
    TObjectPtr<AActor> TargetActor;

protected:
    virtual void Attack() override;

    /**
     * Spawn a bowling ball projectile at the ammo socket location.
     * @return Spawned ammo instance
     */
    TObjectPtr<ABowlingBallTurretAmmo> SpawnAmmo() const;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo")
    TSubclassOf<ABowlingBallTurretAmmo> BowlingBallTurretAmmoClass;

    // Runtime state
    UPROPERTY(Transient)
    TObjectPtr<UBowlingBallTurretAnimInstance> AnimInstance;
};
