#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/ConfigurationValidatable.h"
#include "CurrencyCrystal.h"
#include "CurrencyManagerSettings.generated.h"

/**
 * Configuration data asset for the currency system.
 * Defines available currency crystal types and their properties for spawning and management.
 * Configurable in the editor with automatic validation on startup.
 */
UCLASS()
class DDKNOCKOFF_API
    UCurrencyManagerSettings : public UDataAsset, public IConfigurationValidatable {
    GENERATED_BODY()

public:
    // IConfigurationValidatable Interface
    virtual void ValidateConfiguration() const override;

    // Currency configuration

    UPROPERTY(EditAnywhere, Category = "Currency")
    TArray<TSubclassOf<ACurrencyCrystal>> AvailableCurrencyCrystals;
};
