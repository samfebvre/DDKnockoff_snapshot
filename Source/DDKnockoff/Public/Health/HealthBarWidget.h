#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"

/**
 * Widget for displaying entity health status with visual progress indicator.
 * Provides Blueprint-implementable interface for health bar visualization.
 * Used by HealthBarWidgetComponent for world-space health display.
 */
UCLASS()
class DDKNOCKOFF_API UHealthBarWidget : public UUserWidget {
    GENERATED_BODY()

public:
    /**
     * Update the health bar visual fill amount.
     * @param Percent - Health percentage (0.0 to 1.0)
     */
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
    void UpdateHealthBarFillAmount(float Percent);
};
