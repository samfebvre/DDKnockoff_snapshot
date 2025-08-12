#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/ConfigurationValidatable.h"
#include "WaveData.h"
#include "LevelData.generated.h"

/**
 * Data asset containing wave configuration for a complete level.
 * Defines the sequence of waves, their composition, and timing for level progression.
 * Used by WaveManager to control enemy spawning throughout the level.
 */
UCLASS(BlueprintType, EditInlineNew)
class DDKNOCKOFF_API ULevelData : public UDataAsset, public IConfigurationValidatable {
    GENERATED_BODY()

public:
    // IConfigurationValidatable Interface
    virtual void ValidateConfiguration() const override;

    // Wave configuration

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Data")
    TArray<FWaveData> Waves;
};
