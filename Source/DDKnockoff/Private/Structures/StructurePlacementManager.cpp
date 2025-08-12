#include "Structures/StructurePlacementManager.h"
#include "Engine/Engine.h"
#include "Core/DDKnockoffGameSettings.h"

void UStructurePlacementManager::Initialize() {
    // Load settings asset path from game settings
    const UDDKnockoffGameSettings* GameSettings = UDDKnockoffGameSettings::Get();
    if (GameSettings && !GameSettings->StructurePlacementSettingsAsset.IsNull()) {
        PlacementSettingsAsset = TSoftObjectPtr<UStructurePlacementSettings>(
            GameSettings->StructurePlacementSettingsAsset);
    }

    // Load everything synchronously at initialization
    LoadPlacementSettings();

    // Ensure settings are valid - if not, the game is broken
    ensureAlways(CachedSettings);
}

void UStructurePlacementManager::Deinitialize() {
    CachedSettings = nullptr;
    CachedPreviewMaterial = nullptr;
}

const UStructurePlacementSettings& UStructurePlacementManager::GetPlacementSettings() const {
    // Settings are guaranteed valid after initialization
    return *CachedSettings;
}

const UMaterialInterface& UStructurePlacementManager::GetPreviewMaterial() const {
    // Material is guaranteed valid after initialization
    return *CachedPreviewMaterial;
}

void UStructurePlacementManager::LoadPlacementSettings() {
    ensureAlways(!PlacementSettingsAsset.IsNull());

    // Load settings synchronously - if this fails, the game is broken
    CachedSettings = PlacementSettingsAsset.LoadSynchronous();
    ensureAlways(CachedSettings);

    // Validate configuration after loading
    CachedSettings->ValidateConfiguration();

    UE_LOG(LogTemp, Log, TEXT("StructurePlacementSubsystem: Settings loaded successfully"));

    // Pre-load the preview material if specified
    if (!CachedSettings->PreviewMaterial.IsNull()) {
        CachedPreviewMaterial = CachedSettings->PreviewMaterial.LoadSynchronous();
        ensureAlways(CachedPreviewMaterial);
    }
}
