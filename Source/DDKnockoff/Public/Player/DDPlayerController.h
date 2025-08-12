#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DDPlayerController.generated.h"

class UDDHUD;
/**
 * 
 */
UCLASS()
class DDKNOCKOFF_API ADDPlayerController : public APlayerController {
    GENERATED_BODY()

    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UDDHUD> HUDClass;

    UPROPERTY(Transient)
    TObjectPtr<UDDHUD> HUDWidget;
};
