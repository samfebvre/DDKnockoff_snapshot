#pragma once

#include "CoreMinimal.h"
#include "TestWorldGameMode.h"
#include "Engine/GameInstance.h"
#include "TestWorldGameInstance.generated.h"

UCLASS()
class DDKNOCKOFFTESTS_API UTestWorldGameInstance : public UGameInstance {
    GENERATED_BODY()

public:
    void InitForTest(UWorld& World);

    // We don't care which game mode the base class has created, we always return ours.
    virtual TSubclassOf<AGameModeBase> OverrideGameModeClass(
        TSubclassOf<AGameModeBase> GameModeClass,
        const FString& MapName,
        const FString& Options,
        const FString& Portal) const override;
};
