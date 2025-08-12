#pragma once

#include "CoreMinimal.h"
#include "Core/ManagerBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "Debug/DebugInformationProvider.h"
#include "DebugInformationManager.generated.h"

class UDebugInformationWidget;
class UDebugInformationSettings;

/**
 * Manager for unified debug information display across the game world.
 * Coordinates debug information providers and manages display widgets.
 * Provides centralized debug output for gameplay systems, functional tests, and development tools.
 */
UCLASS()
class DDKNOCKOFF_API UDebugInformationManager : public UManagerBase {
    GENERATED_BODY()

public:
    // ManagerBase Interface
    virtual void Initialize() override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;

    // Provider management

    /**
     * Register a debug information provider for display updates.
     * @param Provider - Provider implementing IDebugInformationProvider interface
     */
    UFUNCTION(BlueprintCallable, Category = "Debug Information")
    void RegisterDebugInformationProvider(
        const TScriptInterface<IDebugInformationProvider>& Provider);

    /**
     * Unregister a debug information provider from display updates.
     * @param Provider - Provider to remove from update list
     */
    UFUNCTION(BlueprintCallable, Category = "Debug Information")
    void UnregisterDebugInformationProvider(
        const TScriptInterface<IDebugInformationProvider>& Provider);

    // Settings management

    /**
     * Set debug information settings for display configuration.
     * @param Settings - Settings data asset for debug display configuration
     */
    UFUNCTION(BlueprintCallable, Category = "Debug Information")
    void SetSettings(UDebugInformationSettings* Settings);

    UFUNCTION(BlueprintCallable, Category = "Debug Information")
    UDebugInformationSettings* GetSettings() const { return DebugSettings; }

    // Widget management

    /**
     * Show or hide the debug information widget.
     * @param bShow - Whether to show the debug widget
     */
    UFUNCTION(BlueprintCallable, Category = "Debug Information")
    void ShowDebugWidget(bool bShow = true);

    /**
     * Check if the debug manager has completed initialization.
     * @return true if fully initialized and ready for use
     */
    bool IsInitialized() const { return bIsInitialized; }

protected:
    // Settings initialization

    /**
     * Load debug settings from configured data asset.
     */
    void LoadSettings();

    /**
     * Check if debug widget should be displayed based on current settings.
     * @return true if widget should be visible
     */
    bool ShouldShowDebugWidget() const;

    // Widget lifecycle

    /**
     * Create and initialize the debug information widget.
     */
    void CreateDebugWidget();

    /**
     * Clean up and destroy the debug widget.
     */
    void DestroyDebugWidget();

    /**
     * Update widget content from all registered providers.
     */
    void UpdateDebugInformationWidget();

private:
    // Provider registry

    UPROPERTY(Transient)
    TArray<TScriptInterface<IDebugInformationProvider>> DebugInformationProviders;

    // Widget management

    UPROPERTY(Transient)
    TObjectPtr<UDebugInformationWidget> DebugInformationWidgetInstance;

    // Configuration

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Debug Information",
        meta = (AllowPrivateAccess = "true"))
    TSoftObjectPtr<UDebugInformationSettings> DebugSettingsAsset;

    UPROPERTY(Transient)
    TObjectPtr<UDebugInformationSettings> DebugSettings;

    // Runtime state

    bool bIsInitialized = false;
    bool bAutoUpdate = true;
    bool bWidgetVisible = false;
    float TimeSinceLastUpdate = 0.0f;

    // Internal debug data

    FString SubsystemDebugCategory;
    FString SubsystemDebugInformation;
};
