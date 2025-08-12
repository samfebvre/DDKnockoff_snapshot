#include "Player/DDPlayerController.h"
#include "UI/DDHUD.h"

void ADDPlayerController::BeginPlay() {
    Super::BeginPlay();

    if (HUDClass) {
        HUDWidget = CreateWidget<UDDHUD>(this, HUDClass);
        if (HUDWidget) { HUDWidget->AddToViewport(); } else {
            UE_LOG(LogTemp,
                   Warning,
                   TEXT("Failed to create HUD widget from class %s"),
                   *HUDClass->GetName());
        }
    } else { UE_LOG(LogTemp, Warning, TEXT("HUDClass is not set in PlayerController")); }
}
