#include "Debug/DebugInformationSettings.h"
#include "Debug/DebugInformationWidget.h"

UDebugInformationSettings::UDebugInformationSettings() {
    // Set default values
    UpdateInterval = 0.0f; // Update every frame by default
    bAutoShowInDevelopment = true;
    bAutoShowInEditor = true;
    bAutoShowInShipping = false;
}

void UDebugInformationSettings::ValidateConfiguration() const {
    // Validate widget class
    ensureAlwaysMsgf(DebugWidgetClass != nullptr,
                     TEXT("DebugInformationSettings: DebugWidgetClass is not set"));

    // Validate update interval is within reasonable bounds
    ensureAlwaysMsgf(UpdateInterval >= 0.0f,
                     TEXT("DebugInformationSettings: UpdateInterval cannot be negative (got %f)"),
                     UpdateInterval);

    ensureAlwaysMsgf(UpdateInterval < 1.0f,
                     TEXT(
                         "DebugInformationSettings: UpdateInterval is very high (%f seconds), debug display will be sluggish"
                     ),
                     UpdateInterval);
}
