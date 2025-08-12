#pragma once

#include "CoreMinimal.h"
#include "TestWorldHelper.h"
#include "Subsystems/EngineSubsystem.h"
#include "TestWorldEngineSubsystem.generated.h"

USTRUCT()
struct FTestWorldData {
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<UGameInstance> GameInstance;

    UPROPERTY()
    TObjectPtr<UWorld> World;
};

UCLASS()
class DDKNOCKOFFTESTS_API UTestWorldEngineSubsystem : public UEngineSubsystem {
    GENERATED_BODY()

    TMap<FName, FTestWorldData> PrivateWorldDatas;

    FTestWorldData SharedWorldData;

    static FTestWorldData MakeTestWorldData(FName Name);

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    virtual void Deinitialize() override;

    FTestWorldHelper GetPrivateWorldHelper();

    FTestWorldHelper GetSharedWorldHelper();

    void DestroyPrivateWorldData(FName Name);
};
