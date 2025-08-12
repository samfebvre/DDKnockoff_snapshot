#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/ConfigurationValidatable.h"
#include "CurrencyCrystal.h"
#include "CurrencySpawner.generated.h"

/**
 * Component for spawning currency crystals with customizable burst patterns.
 * Handles directional spawning with configurable spread angles and speed variations.
 * Provides burst spawning functionality for reward drops and enemy death effects.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DDKNOCKOFF_API UCurrencySpawner : public USceneComponent, public IConfigurationValidatable {
    GENERATED_BODY()

public:
    UCurrencySpawner();

    // Component lifecycle
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime,
                               ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // Currency spawning

    /**
     * Spawn multiple currency crystals in a directional burst pattern.
     * @param CrystalClass - Type of currency crystal to spawn
     * @param Count - Number of crystals to spawn
     * @param Direction - Base direction for the burst
     * @param Speed - Base speed for launched crystals
     */
    UFUNCTION(BlueprintCallable, Category = "Currency")
    void SpawnCurrencyBurst(TSubclassOf<ACurrencyCrystal> CrystalClass,
                            int32 Count,
                            FVector Direction,
                            float Speed) const;

    /**
     * Get upward-forward vector for consistent currency launch direction.
     * @return Normalized vector combining up and forward directions
     */
    FVector GetUpForwardVector() const;

    // IConfigurationValidatable Interface
    virtual void ValidateConfiguration() const override;

protected:
    // Burst configuration

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Burst")
    float SpreadAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Burst")
    float VerticalSpreadAngle = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency|Burst")
    float SpeedVariation = 0.2f;
};
