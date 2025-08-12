#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "HealthBarWidgetComponent.generated.h"

/**
 * Widget component for displaying health bars in world space.
 * Automatically manages health bar widget positioning and visibility.
 * Connects to HealthComponent for automatic health status updates.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DDKNOCKOFF_API UHealthBarWidgetComponent : public UWidgetComponent {
    GENERATED_BODY()

public:
    UHealthBarWidgetComponent();

    // Component lifecycle
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime,
                               ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;
};
