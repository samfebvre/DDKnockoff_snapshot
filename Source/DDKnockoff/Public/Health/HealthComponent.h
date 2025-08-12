#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/ConfigurationValidatable.h"
#include "HealthComponent.generated.h"

class UHealthBarWidget;
class UHealthBarWidgetComponent;

/**
 * Component that manages entity health, damage processing, and health bar UI.
 * Provides automatic health bar visibility management and death event handling.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DDKNOCKOFF_API UHealthComponent : public UActorComponent, public IConfigurationValidatable {
    GENERATED_BODY()

public:
    UHealthComponent();

    // Health management

    /**
     * Apply damage to this entity, clamping to zero and triggering death events.
     * @param Amount - Damage amount to apply
     */
    UFUNCTION(BlueprintCallable, Category="Health")
    void TakeDamage(float Amount);

    /**
     * Check if this entity has reached zero health.
     * @return true if CurrentHealth <= 0
     */
    UFUNCTION(BlueprintCallable, Category="Health")
    bool IsDead() const;

    /**
     * Get the current health value.
     * @return Current health amount
     */
    UFUNCTION(BlueprintCallable, Category="Health")
    float GetCurrentHealth() const { return CurrentHealth; }

    /**
     * Get the maximum health value.
     * @return Maximum health amount
     */
    UFUNCTION(BlueprintCallable, Category="Health")
    float GetMaxHealth() const { return MaxHealth; }

#if WITH_EDITOR || UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
    /**
     * Set current health directly - FOR TESTING ONLY.
     * @param NewHealth - New health value to set
     */
    void SetCurrentHealthForTesting(float NewHealth) {
        CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
    }
#endif

    // UI management

    /**
     * Manually control health bar visibility.
     * @param bVisible - Whether health bar should be visible
     */
    UFUNCTION(BlueprintCallable, Category="UI")
    void SetHealthBarVisibility(bool bVisible) const;

    /**
     * Update the health bar fill percentage based on current health.
     */
    UFUNCTION()
    void UpdateHealthBarFillAmount() const;

    // Events

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

    UPROPERTY(BlueprintAssignable)
    FOnDeath OnReachedZeroHealth;

    // IConfigurationValidatable Interface Implementation
    virtual void ValidateConfiguration() const override;

protected:
    virtual void OnRegister() override;
    virtual void BeginPlay() override;

    // Health configuration

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health")
    float MaxHealth = 100.0f;

    UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category="Health")
    float CurrentHealth;

    // UI configuration

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="UI")
    TObjectPtr<UHealthBarWidgetComponent> HealthBarWidgetComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
    TSubclassOf<UHealthBarWidget> HealthBarWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
    bool bHideHealthBarWhenFullHealth = true;

private:
    /**
     * Update health bar visibility based on current health and configuration.
     */
    void UpdateHealthBarVisibility();
};
