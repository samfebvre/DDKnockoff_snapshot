#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Entities/FactionEnums.h"
#include "Core/ConfigurationValidatable.h"
#include "EnemyDetectionComponent.generated.h"

/**
 * Result structure for enemy detection operations.
 */
USTRUCT(BlueprintType)
struct FEnemyDetectionResult {
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Detection")
    bool bEnemiesInRange = false;

    UPROPERTY(BlueprintReadOnly, Category = "Detection")
    TObjectPtr<AActor> ClosestEnemy = nullptr;
};

/**
 * Component that detects enemy entities within a configurable radius and optional vision cone.
 * Used by defensive structures and AI characters for target acquisition.
 * Supports both omnidirectional and directional detection modes.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DDKNOCKOFF_API UEnemyDetectionComponent
    : public UActorComponent, public IConfigurationValidatable {
    GENERATED_BODY()

public:
    UEnemyDetectionComponent();

    virtual void BeginPlay() override;

    /**
     * Perform enemy detection and return the closest valid target.
     * @return Detection result with closest enemy if found
     */
    FEnemyDetectionResult DetectEnemies();

    // IConfigurationValidatable Interface Implementation
    virtual void ValidateConfiguration() const override;

    /**
     * Test helper - allows tests to access detection sphere for setup.
     * @return Detection sphere component
     */
    USphereComponent* GetDetectionSphere() const { return DetectionSphere; }

    /**
     * Get the detection radius for this component.
     * @return Detection radius in world units
     */
    float GetDetectionRadius() const { return DetectionRadius; }

#if WITH_EDITOR || UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
    /**
     * Set the detection radius for testing purposes.
     * @param NewRadius The new detection radius
     */
    void SetDetectionRadiusForTesting(float NewRadius) { DetectionRadius = NewRadius; }
#endif

protected:
    virtual void OnRegister() override;

    // Detection configuration

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionRadius = 500.0f;

    UPROPERTY(EditAnywhere,
        BlueprintReadWrite,
        Category = "Vision Cone",
        meta = (ClampMin = "1.0", ClampMax = "360.0"))
    float VisionConeAngleDegrees = 360.0f;

    // Components

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Detection")
    TObjectPtr<USphereComponent> DetectionSphere;

    // Runtime state

    UPROPERTY(Transient)
    TArray<AActor*> ActorsInRange;

private:
    /**
     * Check for enemies within detection range and find the closest one.
     * @param OutClosestEnemy - Reference to store the closest enemy found
     * @return true if enemies were found
     */
    bool CheckForEnemiesInRange(TObjectPtr<AActor>& OutClosestEnemy) const;

    /**
     * Get the faction of the component's owner for enemy identification.
     * @return Owner's faction
     */
    EFaction GetOwnerFaction() const;

    /**
     * Check if an actor is within the configured vision cone.
     * @param Actor - Actor to check
     * @return true if actor is within vision cone
     */
    bool IsActorInVisionCone(AActor* Actor) const;
};
