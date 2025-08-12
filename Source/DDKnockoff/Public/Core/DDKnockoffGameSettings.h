#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Core/ConfigurationValidatable.h"
#include "DDKnockoffGameSettings.generated.h"

/**
 * Global game configuration settings for DDKnockoff project.
 * Provides centralized configuration for character classes, data assets, and system paths.
 * Settings are automatically loaded from Game.ini and validated on startup.
 */
UCLASS(Config=Game, DefaultConfig, DisplayName="DDKnockoff Game Settings")
class DDKNOCKOFF_API UDDKnockoffGameSettings : public UObject, public IConfigurationValidatable {
    GENERATED_BODY()

public:
    UDDKnockoffGameSettings();

    /**
     * Get the singleton game settings instance.
     * @return Current game settings configuration
     */
    static const UDDKnockoffGameSettings* Get();

    // IConfigurationValidatable Interface
    virtual void ValidateConfiguration() const override;

    // Character configuration

    UPROPERTY(Config,
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Character Settings",
        meta = (DisplayName = "Default Player Character Class"))
    FString DefaultPlayerCharacterClass;

    // Data asset paths

    UPROPERTY(Config,
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Data Asset Settings",
        meta = (DisplayName = "Debug Information Settings Asset"))
    FSoftObjectPath DebugInformationSettingsAsset;

    UPROPERTY(Config,
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Data Asset Settings",
        meta = (DisplayName = "Wave Manager Settings Asset"))
    FSoftObjectPath WaveManagerSettingsAsset;

    UPROPERTY(Config,
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Data Asset Settings",
        meta = (DisplayName = "Structure Placement Settings Asset"))
    FSoftObjectPath StructurePlacementSettingsAsset;

    // System class configuration

    UPROPERTY(Config,
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Navigation Settings",
        meta = (DisplayName = "Defense Nav Area Class"))
    FString DefenseNavAreaClass;

    UPROPERTY(Config,
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Weapon Settings",
        meta = (DisplayName = "Ballista Ammo Class"))
    FString BallistaAmmoClass;

    UPROPERTY(Config,
        EditAnywhere,
        BlueprintReadOnly,
        Category = "UI Settings",
        meta = (DisplayName = "Health Bar Widget Class"))
    FString HealthBarWidgetClass;

    // Asset paths

    UPROPERTY(Config,
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Asset Paths",
        meta = (DisplayName = "Defense Structures Path"))
    FString DefenseStructuresPath;
};
