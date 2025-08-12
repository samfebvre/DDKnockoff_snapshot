#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CurrencyCrystal.h"
#include "CurrencyManagerSettings.h"
#include "Core/ManagerBase.h"
#include "CurrencyManager.generated.h"

/**
 * Result structure for currency crystal spawning calculations.
 * Specifies which crystal types and quantities should be spawned for a given currency amount.
 */
USTRUCT(BlueprintType)
struct FCurrencySpawnInfo {
    GENERATED_BODY()

    /** Type of currency crystal to spawn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
    TSubclassOf<ACurrencyCrystal> CrystalClass;

    /** Number of crystals of this type to spawn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
    int32 Count = 0;
};

/**
 * Manager for currency crystal spawning and distribution calculations.
 * Handles the conversion of currency amounts into appropriate crystal combinations
 * based on configured crystal types and values.
 */
UCLASS()
class DDKNOCKOFF_API UCurrencyManager : public UManagerBase {
    GENERATED_BODY()

public:
    UCurrencyManager();

    // ManagerBase Interface
    virtual void Initialize() override;
    virtual void Deinitialize() override;

    // Currency calculation

    /**
     * Calculate optimal crystal distribution for a given currency amount.
     * Uses greedy algorithm to minimize total crystal count while meeting minimum requirements.
     * @param TotalCurrencyAmount - Total currency value to represent
     * @param MinimumCrystalCount - Minimum number of crystals that must be spawned
     * @return Array of crystal types and counts to spawn
     */
    UFUNCTION(BlueprintCallable, Category = "Currency")
    TArray<FCurrencySpawnInfo> CalculateCurrencySpawnInfo(int32 TotalCurrencyAmount,
                                                          int32 MinimumCrystalCount) const;

    // Configuration

    /**
     * Update the currency manager with new settings configuration.
     * @param NewSettings - New settings to apply
     */
    UFUNCTION(BlueprintCallable, Category = "Currency")
    void SetSettings(UCurrencyManagerSettings* NewSettings);

private:
    // Configuration

    UPROPERTY()
    TObjectPtr<UCurrencyManagerSettings> Settings;

    // Utility functions

    /**
     * Sort crystal types by currency value from highest to lowest for optimal distribution.
     */
    void SortCrystalTypes() const;
};
