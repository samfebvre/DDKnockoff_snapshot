#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Entities/Entity.h"
#include "Core/DependencyInjectable.h"
#include "Damage/DamagePayload.h"
#include "MockEnemy.generated.h"

class UHealthComponent;
class UCapsuleComponent;
class UEntityData;
class IEntityManagerInterface;

UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EMockEnemyFlags : uint8 {
    None = 0,
    Invulnerable = 1 << 0,
    // Ignores damage (doesn't take damage at all)
    Immortal = 1 << 1,
    // Ignores death (can take damage but won't die/destroy)
};

ENUM_CLASS_FLAGS(EMockEnemyFlags);

UENUM(BlueprintType, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EMockEnemyState : uint8 {
    None = 0,
    HasBeenDamaged = 1 << 0,
    // Has received damage at least once
};

ENUM_CLASS_FLAGS(EMockEnemyState);

UCLASS()
class DDKNOCKOFFTESTS_API AMockEnemy : public AActor, public IEntity, public IDependencyInjectable {
    GENERATED_BODY()

public:
    AMockEnemy();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Core components needed for targeting and damage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UHealthComponent> HealthComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UCapsuleComponent> CapsuleComponent;

    // Entity data
    UPROPERTY(Transient, Instanced)
    TObjectPtr<UEntityData> EntityData;

    // Dependency injection
    UPROPERTY(Transient)
    UEntityManager* EntityManager;

    // Test configuration for faction override
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Configuration")
    EFaction TestFaction = EFaction::Enemy;

public:
    // IEntity interface implementation
    virtual void TakeDamage(const FDamagePayload& DamagePayload) override;
    virtual void OnHitboxBeginNotifyReceived(UAnimSequenceBase* Animation) override {}
    virtual void OnHitboxEndNotifyReceived(UAnimSequenceBase* Animation) override {}
    virtual void TakeKnockback(const FVector& Direction, const float Strength) override {}
    virtual EFaction GetFaction() const override { return TestFaction; }
    virtual AActor* GetActor() override { return this; }
    virtual float GetHalfHeight() const override;
    virtual float GetRadius() const;
    virtual UEntityData* GetEntityData() const override { return EntityData; }
    virtual EEntityType GetEntityType() const override { return EEntityType::Character; }
    virtual bool IsCurrentlyTargetable() const override { return true; }
    // End IEntity interface

    // IDependencyInjectable interface
    virtual bool HasRequiredDependencies() const override;
    virtual void CollectDependencies() override;
    // End IDependencyInjectable interface

    // Death event handler
    UFUNCTION()
    void OnReachedZeroHealth();

    // Test helpers
    void SetHealth(float NewHealth) const;
    float GetCurrentHealth() const;
    bool IsDead() const;
    void SetFaction(EFaction InFaction);

    // Test configuration
    UPROPERTY(EditAnywhere,
        BlueprintReadWrite,
        Category = "Test Configuration",
        meta = (Bitmask, BitmaskEnum = "/Script/DDKnockoffTests.EMockEnemyFlags"))
    uint8 TestFlags = 0;

    // Helper functions for flag management
    FORCEINLINE bool HasTestFlag(EMockEnemyFlags Flag) const {
        return (TestFlags & static_cast<uint8>(Flag)) != 0;
    }

    FORCEINLINE void SetTestFlag(EMockEnemyFlags Flag) { TestFlags |= static_cast<uint8>(Flag); }
    FORCEINLINE void ClearTestFlag(EMockEnemyFlags Flag) { TestFlags &= ~static_cast<uint8>(Flag); }

    // Damage tracking for tests
    UPROPERTY(Transient, BlueprintReadOnly, Category = "Test State")
    FDamagePayload LastDamagePayload;

    // State tracking
    UPROPERTY(Transient, BlueprintReadOnly, Category = "Test State")
    uint8 StateFlags = 0;

    // State helper functions
    FORCEINLINE bool HasState(EMockEnemyState State) const {
        return (StateFlags & static_cast<uint8>(State)) != 0;
    }

    FORCEINLINE void SetState(EMockEnemyState State) { StateFlags |= static_cast<uint8>(State); }
    FORCEINLINE void ClearState(EMockEnemyState State) { StateFlags &= ~static_cast<uint8>(State); }

    // Damage query helpers
    bool WasDamagedBy(const AActor* Actor) const;
    bool WasDamagedByClass(const TSubclassOf<AActor>& ActorClass) const;
};
