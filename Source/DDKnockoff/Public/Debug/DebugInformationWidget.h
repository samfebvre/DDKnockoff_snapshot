#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DebugInformationWidget.generated.h"

/**
 * Widget for displaying debug information on screen.
 * Provides Blueprint-implementable interface for debug text display.
 * Used by DebugInformationManager for consolidated debug output.
 */
UCLASS()
class DDKNOCKOFF_API UDebugInformationWidget : public UUserWidget {
    GENERATED_BODY()

public:
    /**
     * Update the debug information display with new text.
     * @param DebugInfo - Formatted debug information text to display
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Debug")
    void UpdateDebugInformation(const FText& DebugInfo);
};
