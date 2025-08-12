#pragma once

#include "CoreMinimal.h"
#include "Core/ManagerBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Structures/StructurePlacementSettings.h"
#include "StructurePlacementManager.generated.h"

/**
 * Manager for structure placement settings and preview material caching.
 * Ensures placement configuration is loaded once and efficiently accessible throughout the game session.
 * Provides centralized access to placement validation rules and visual feedback materials.
 */
UCLASS(config = Game)
class DDKNOCKOFF_API UStructurePlacementManager : public UManagerBase {
    GENERATED_BODY()

public:
    // ManagerBase Interface
    virtual void Initialize() override;
    virtual void Deinitialize() override;

    // Settings access

    /**
     * Get the structure placement settings data asset.
     * Guaranteed to be valid after successful initialization.
     * @return Reference to placement settings
     */
    const UStructurePlacementSettings& GetPlacementSettings() const;

    /**
     * Get the preview material for structure placement feedback.
     * Used for visual indication of placement validity.
     * @return Reference to preview material
     */
    const UMaterialInterface& GetPreviewMaterial() const;

    /**
     * Check if settings have been successfully loaded and cached.
     * @return true if settings are ready for use
     */
    UFUNCTION(BlueprintCallable, Category = "Structure Placement")
    bool AreSettingsLoaded() const { return CachedSettings != nullptr; }

protected:
    // Configuration

    UPROPERTY(config,
        EditAnywhere,
        Category = "Settings",
        meta = (DisplayName = "Placement Settings Asset"))
    TSoftObjectPtr<UStructurePlacementSettings> PlacementSettingsAsset;

private:
    /**
     * Load the placement settings data asset synchronously during initialization.
     */
    void LoadPlacementSettings();

    // Cached data

    UPROPERTY(Transient)
    const UStructurePlacementSettings* CachedSettings;

    UPROPERTY(Transient)
    UMaterialInterface* CachedPreviewMaterial;
};
