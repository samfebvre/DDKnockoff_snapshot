#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/ConfigurationValidatable.h"
#include "WaveManagerSettings.generated.h"

class ULevelData;

/**
 * Configuration data asset for wave management system.
 * Defines timing parameters and level data for wave-based enemy spawning.
 * Configurable in the editor with automatic validation on startup.
 */
UCLASS(BlueprintType, meta = (DisplayName = "Wave Manager Settings"))
class DDKNOCKOFF_API UWaveManagerSettings : public UDataAsset, public IConfigurationValidatable {
    GENERATED_BODY()

public:
    // IConfigurationValidatable Interface
    virtual void ValidateConfiguration() const override;

    // Wave timing configuration

    UPROPERTY(EditAnywhere, Category = "Wave Settings", meta = (AllowPrivateAccess = "true"))
    float WaveCountdownDuration;

    UPROPERTY(EditAnywhere, Category = "Wave Settings", meta = (AllowPrivateAccess = "true"))
    float TimeBetweenSpawns;

    // Level data configuration

    UPROPERTY(EditAnywhere, Category = "Wave Settings", meta = (AllowPrivateAccess = "true"))
    ULevelData* LevelData;
};
