#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/ConfigurationValidatable.h"
#include "DebugInformationSettings.generated.h"

class UDebugInformationWidget;

/**
 * Data asset containing all settings for the debug information display system.
 * This centralizes configuration for widget class, update frequency, and other debug-related settings.
 */
UCLASS(BlueprintType, meta = (DisplayName = "Debug Information Settings"))
class DDKNOCKOFF_API
    UDebugInformationSettings : public UDataAsset, public IConfigurationValidatable {
    GENERATED_BODY()

public:
    UDebugInformationSettings();

    // Widget class used for debug information display
    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Widget",
        meta = (DisplayName = "Debug Widget Class"))
    TSubclassOf<UDebugInformationWidget> DebugWidgetClass;

    // How often to update the debug display (in seconds). 0 = every frame
    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Performance",
        meta = (DisplayName = "Update Interval", ClampMin = "0.0", ClampMax = "1.0"))
    float UpdateInterval;

    // Whether to show debug widget automatically in development builds
    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Display",
        meta = (DisplayName = "Auto Show In Development"))
    bool bAutoShowInDevelopment;

    // Whether to show debug widget automatically in editor
    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Display",
        meta = (DisplayName = "Auto Show In Editor"))
    bool bAutoShowInEditor;

    // Whether to show debug widget automatically in shipping builds
    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Display",
        meta = (DisplayName = "Auto Show In Shipping"))
    bool bAutoShowInShipping;

    // IConfigurationValidatable interface
    virtual void ValidateConfiguration() const override;
};
