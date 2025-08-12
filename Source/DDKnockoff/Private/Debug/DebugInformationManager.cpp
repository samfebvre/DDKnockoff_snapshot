#include "Debug/DebugInformationManager.h"
#include "Debug/DebugInformationWidget.h"
#include "Debug/DebugInformationSettings.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"
#include "Core/DDKnockoffGameSettings.h"

void UDebugInformationManager::Initialize() {
    bIsInitialized = true;
    bAutoUpdate = true;
    bWidgetVisible = false;
    TimeSinceLastUpdate = 0.0f;

    // Load settings from data asset
    LoadSettings();

    // Register ourselves as a debug provider to show subsystem info
    RegisterDebugInformationProvider(this);

    // Auto-show widget if configured to do so
    if (ShouldShowDebugWidget()) { ShowDebugWidget(true); }

    UE_LOG(LogTemp, Log, TEXT("DebugInformationSubsystem initialized"));
}

void UDebugInformationManager::Deinitialize() {
    // Clean up widget
    DestroyDebugWidget();

    // Clear providers
    DebugInformationProviders.Empty();

    bIsInitialized = false;

    UE_LOG(LogTemp, Log, TEXT("DebugInformationSubsystem deinitialized"));
}

void UDebugInformationManager::RegisterDebugInformationProvider(
    const TScriptInterface<IDebugInformationProvider>& Provider) {
    if (Provider.GetInterface() && !DebugInformationProviders.Contains(Provider)) {
        DebugInformationProviders.Add(Provider);
        UE_LOG(LogTemp,
               VeryVerbose,
               TEXT("Debug provider registered: %s"),
               Provider.GetInterface() ? *Provider->GetDebugCategory() : TEXT("Unknown"));
    }
}

void UDebugInformationManager::UnregisterDebugInformationProvider(
    const TScriptInterface<IDebugInformationProvider>& Provider) {
    if (Provider.GetInterface()) {
        DebugInformationProviders.Remove(Provider);
        UE_LOG(LogTemp,
               VeryVerbose,
               TEXT("Debug provider unregistered: %s"),
               *Provider->GetDebugCategory());
    }
}

void UDebugInformationManager::SetSettings(UDebugInformationSettings* Settings) {
    if (DebugSettings != Settings) {
        DebugSettings = Settings;

        if (DebugSettings) { DebugSettings->ValidateConfiguration(); }
    }
}

void UDebugInformationManager::ShowDebugWidget(bool bShow) {
    if (bShow && !bWidgetVisible) {
        CreateDebugWidget();
        bWidgetVisible = true;
    } else if (!bShow && bWidgetVisible) {
        DestroyDebugWidget();
        bWidgetVisible = false;
    }
}

void UDebugInformationManager::Tick(float DeltaTime) {
    if (!bIsInitialized || !bAutoUpdate || !DebugSettings) { return; }

    TimeSinceLastUpdate += DeltaTime;

    // Check if it's time to update
    if (TimeSinceLastUpdate >= DebugSettings->UpdateInterval) {
        UpdateDebugInformationWidget();
        TimeSinceLastUpdate = 0.0f;
    }
}

void UDebugInformationManager::LoadSettings() {
    // Load settings path from game settings
    const UDDKnockoffGameSettings* GameSettings = UDDKnockoffGameSettings::Get();
    if (GameSettings && !GameSettings->DebugInformationSettingsAsset.IsNull()) {
        DebugSettingsAsset = TSoftObjectPtr<UDebugInformationSettings>(
            GameSettings->DebugInformationSettingsAsset);

        if (DebugSettingsAsset.IsValid() || DebugSettingsAsset.IsPending()) {
            DebugSettings = DebugSettingsAsset.LoadSynchronous();
            if (DebugSettings) {
                DebugSettings->ValidateConfiguration();
                UE_LOG(LogTemp,
                       Log,
                       TEXT("DebugInformationSubsystem: Loaded settings from %s"),
                       *GameSettings->DebugInformationSettingsAsset.ToString());
            } else {
                UE_LOG(LogTemp,
                       Warning,
                       TEXT("DebugInformationSubsystem: Failed to load settings from %s"),
                       *GameSettings->DebugInformationSettingsAsset.ToString());
            }
        } else {
            UE_LOG(LogTemp,
                   Warning,
                   TEXT("DebugInformationSubsystem: Settings asset not found at %s"),
                   *GameSettings->DebugInformationSettingsAsset.ToString());
        }
    } else {
        UE_LOG(LogTemp,
               Warning,
               TEXT(
                   "DebugInformationSubsystem: No debug information settings asset configured in game settings"
               ));
    }
}

bool UDebugInformationManager::ShouldShowDebugWidget() const {
    if (!DebugSettings) { return false; }

    if (GEngine && !GEngine->GameViewport) { return false; }

#if UE_BUILD_SHIPPING
    return DebugSettings->bAutoShowInShipping;
#elif UE_BUILD_DEVELOPMENT || UE_BUILD_DEBUG
    return DebugSettings->bAutoShowInDevelopment;
#elif WITH_EDITOR
    return DebugSettings->bAutoShowInEditor;
#else
    return false;
#endif
}

void UDebugInformationManager::CreateDebugWidget() {
    if (!DebugSettings || !DebugSettings->DebugWidgetClass) {
        UE_LOG(LogTemp,
               Warning,
               TEXT("DebugInformationSubsystem: No widget class set in settings"));
        return;
    }

    if (DebugInformationWidgetInstance) {
        UE_LOG(LogTemp, Warning, TEXT("DebugInformationSubsystem: Widget already exists"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World) {
        UE_LOG(LogTemp, Error, TEXT("DebugInformationSubsystem: No valid world"));
        return;
    }

    DebugInformationWidgetInstance = CreateWidget<UDebugInformationWidget>(
        World,
        DebugSettings->DebugWidgetClass);

    if (DebugInformationWidgetInstance) {
        DebugInformationWidgetInstance->AddToViewport();
        UE_LOG(LogTemp,
               Log,
               TEXT("DebugInformationSubsystem: Debug widget created and added to viewport"));
    } else {
        UE_LOG(LogTemp, Error, TEXT("DebugInformationSubsystem: Failed to create debug widget"));
    }
}

void UDebugInformationManager::DestroyDebugWidget() {
    if (DebugInformationWidgetInstance) {
        DebugInformationWidgetInstance->RemoveFromParent();
        DebugInformationWidgetInstance = nullptr;
        UE_LOG(LogTemp, Log, TEXT("DebugInformationSubsystem: Debug widget destroyed"));
    }
}

void UDebugInformationManager::UpdateDebugInformationWidget() {
    if (!DebugInformationWidgetInstance) {
        return; // No widget to update
    }

    FString DebugInfoString;

    for (const TScriptInterface<IDebugInformationProvider>& Provider : DebugInformationProviders) {
        if (!Provider.GetInterface()) { continue; }

        const FString ProviderInfo = Provider->GetDebugInformation();

        if (!ProviderInfo.IsEmpty()) {
            if (!DebugInfoString.IsEmpty()) { DebugInfoString.Append(TEXT("\n\n")); }

            const FString Category = Provider->GetDebugCategory();
            DebugInfoString.Append(
                FString::Printf(TEXT("<header>%s</>\n%s"), *Category, *ProviderInfo));
        }
    }

    DebugInformationWidgetInstance->UpdateDebugInformation(FText::FromString(DebugInfoString));
}
