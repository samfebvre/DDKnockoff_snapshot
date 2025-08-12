#pragma once

#include "CoreMinimal.h"
#include "Structures/DefensiveStructure.h"
#include "SliceAndDiceEnums.h"
#include "SliceAndDice.generated.h"

class USliceAndDiceAnimInstance;
class UEnemyDetectionComponent;

/**
 * Rotating blade defensive structure that spins up when enemies are detected.
 * Features variable speed rotation with damage scaling based on blade velocity.
 * Uses dynamic hitbox activation and hit timing to prevent spam damage.
 */
UCLASS()
class DDKNOCKOFF_API ASliceAndDice : public ADefensiveStructure {
    GENERATED_BODY()

public:
    ASliceAndDice();

    // Actor lifecycle
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // IConfigurationValidatable Interface Implementation
    virtual void ValidateConfiguration() const override;

    // Test access methods

    /**
     * Get the enemy detection component for this slice and dice.
     * @return Enemy detection component
     */
    UEnemyDetectionComponent* GetEnemyDetectionComponent() const { return EnemyDetectionComponent; }

    /**
     * Get the current blade rotation velocity.
     * @return Current blade rotation velocity
     */
    float GetCurrentBladeRotationVelocity() const { return CurrentBladeRotationVelocity; }

    /**
     * Get the blade radius for damage detection.
     * @return Blade radius
     */
    float GetBladeRadius() const { return BladeRadius; }

    /**
     * Get the current hitbox state.
     * @return Current hitbox state
     */
    ESliceAndDiceHitboxState GetHitboxState() const { return HitboxState; }

protected:
    // State management

    /**
     * Update the overall structure state and blade behavior.
     */
    void UpdateStructureState();

    /**
     * Update blade rotation speed and angle based on enemy presence.
     * @param DeltaTime - Time elapsed since last update
     */
    void UpdateBladeRotation(float DeltaTime);

    /**
     * Update blade animation parameters for visual feedback.
     */
    void UpdateBladeAnimation() const;

    /**
     * Scale hitbox size based on current blade rotation speed.
     */
    void UpdateHitboxScale() const;

    /**
     * Enable or disable hitbox collision based on rotation threshold.
     */
    void UpdateHitboxState();

    // Combat system

    /**
     * Process hit detection and damage application with timing controls.
     */
    void HandleHitDetection();

    /**
     * Calculate delay between hits based on blade speed.
     * @return Delay time in seconds
     */
    float CalculateHitDelay() const;

    /**
     * Process all current hitbox overlaps and apply damage.
     */
    void ProcessHitboxOverlaps();

    // Components

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> HitboxMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UEnemyDetectionComponent> EnemyDetectionComponent;

    // Configuration

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SliceAndDice")
    float BladeRotationAcceleration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SliceAndDice")
    float BladeRotationMaxSpeed = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SliceAndDice")
    float BladeRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SliceAndDice")
    double MinEnemyHitDelay = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SliceAndDice")
    double MaxEnemyHitDelay = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SliceAndDice")
    float HitboxEnabledThreshold = 0.5f;

    // Runtime state

    UPROPERTY(Transient)
    TObjectPtr<USliceAndDiceAnimInstance> AnimInstance;

    UPROPERTY(Transient)
    float BladeRotation = 0.0f;

    UPROPERTY(Transient)
    float CurrentBladeRotationVelocity = 0.0f;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "State")
    ESliceAndDiceHitboxState HitboxState = ESliceAndDiceHitboxState::Disabled;

    double LastEnemyHitTime = 0.0f;
};
