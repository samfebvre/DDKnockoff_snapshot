#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/ConfigurationValidatable.h"
#include "CurrencyCrystal.generated.h"

/**
 * Currency crystal actor with physics-based attraction and collection mechanics.
 * Features dynamic attraction to player with configurable force curves and rotation behavior.
 * Represents collectible currency with visual feedback and physics simulation.
 */
UCLASS()
class DDKNOCKOFF_API ACurrencyCrystal : public AActor, public IConfigurationValidatable {
    GENERATED_BODY()

public:
    ACurrencyCrystal();

    // Actor lifecycle
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Access methods

    /**
     * Get the crystal mesh component for external manipulation.
     * @return Crystal mesh component
     */
    UStaticMeshComponent* GetMesh() const;

    /**
     * Get current attraction resistance based on time since spawn.
     * @return Resistance factor (0.0 = no resistance, 1.0 = full resistance)
     */
    float GetCurrentAttractionSubjectivity() const;

    /**
     * Get the currency value of this crystal.
     * @return Currency amount this crystal is worth
     */
    int GetCurrencyAmount() const { return CurrencyAmount; }

    // Attraction system

    /**
     * Update attraction behavior toward target location.
     * @param TargetLocation - World position to attract toward
     */
    void UpdateAttraction(const FVector& TargetLocation);

    /**
     * Update rotational motion and visual effects.
     */
    void UpdateRotation();

    // IConfigurationValidatable Interface Implementation
    virtual void ValidateConfiguration() const override;

protected:
    /**
     * Apply attraction forces based on target direction and strength.
     * @param ForceDirection - Normalized direction toward target
     * @param AttractionStrength - Force multiplier for this frame
     */
    void ApplyAttractionForces(const FVector& ForceDirection, float AttractionStrength) const;

    /**
     * Apply rotational forces for visual spinning effect.
     * @param RotationStrength - Rotation force multiplier
     */
    void ApplyRotationForces(float RotationStrength) const;

    // Components

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CrystalMesh;

    // Configuration

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
    int CurrencyAmount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attraction")
    UCurveFloat* AttractionResistanceCurve;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attraction")
    float AttractionForceStrength = 10000.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attraction")
    float UpwardForceStrength = 980.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attraction")
    float RotationForceStrength = 1000000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attraction")
    float VelocityLerpStrength = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attraction")
    float RotationAxisAdjustmentRange = 0.1f;

    // Runtime state

    double SpawnTimestamp;
    FVector CurrentRotationAxis;
};
