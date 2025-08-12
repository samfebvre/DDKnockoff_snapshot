#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DebugInformationProvider.generated.h"

UINTERFACE()
class DDKNOCKOFF_API UDebugInformationProvider : public UInterface {
    GENERATED_BODY()
};

/**
 * Interface for objects that provide debug information to the debug system.
 * Implementing classes can contribute categorized debug data for display in debug widgets.
 * Used by managers, components, and other systems to expose runtime information.
 */
class DDKNOCKOFF_API IDebugInformationProvider {
    GENERATED_BODY()

public:
    /**
     * Get the category name for organizing debug information display.
     * @return Category name for this provider (e.g., "Wave Manager", "Currency System")
     */
    virtual FString GetDebugCategory() const = 0;

    /**
     * Get current debug information string for display.
     * @return Formatted debug information text
     */
    virtual FString GetDebugInformation() const = 0;
};
