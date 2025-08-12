#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "Core/ConfigurationValidatable.h"
#include "StructurePlacementSettings.generated.h"

/**
 * Data asset containing all settings for structure placement preview system.
 * This centralizes configuration for colors, materials, and other placement-related settings.
 */
UCLASS(BlueprintType, meta = (DisplayName = "Structure Placement Settings"))
class DDKNOCKOFF_API UStructurePlacementSettings
    : public UDataAsset, public IConfigurationValidatable {
    GENERATED_BODY()

public:
    UStructurePlacementSettings();

    // Preview material used for structure placement
    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Preview Material",
        meta = (DisplayName = "Preview Material"))
    TSoftObjectPtr<UMaterialInterface> PreviewMaterial;

    // Color used when structure placement is valid
    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Preview Colors",
        meta = (DisplayName = "Valid Placement Color"))
    FLinearColor ValidPlacementColor;

    // Color used when structure placement is partially invalid (e.g., only distance issue)
    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Preview Colors",
        meta = (DisplayName = "Partially Invalid Placement Color"))
    FLinearColor PartiallyInvalidPlacementColor;

    // Color used when structure placement is invalid
    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Preview Colors",
        meta = (DisplayName = "Invalid Placement Color"))
    FLinearColor InvalidPlacementColor;

    // Maximum distance from camera for valid structure placement
    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Placement Rules",
        meta = (DisplayName = "Max Placement Distance", ClampMin = "100.0", ClampMax = "10000.0"))
    float MaxPlacementDistance;

    // Rotation speed during positioning phase
    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Input Settings",
        meta = (DisplayName = "Positioning Phase Rotation Speed", ClampMin = "1.0", ClampMax =
            "50.0"))
    float PositioningPhaseRotationSpeed;

    // Rotation speed during rotation phase
    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Input Settings",
        meta = (DisplayName = "Rotation Phase Rotation Speed", ClampMin = "1.0", ClampMax = "20.0"))
    float RotationPhaseRotationSpeed;

    // IConfigurationValidatable interface
    virtual void ValidateConfiguration() const override;
};
