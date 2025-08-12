#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Debug/DebugInformationProvider.h"
#include "FunctionalTestDebugComponent.generated.h"

class UDebugInformationManager;
class AFunctionalTest;

/**
 * Component that provides debug information display for functional tests.
 * Add this component to any AFunctionalTest to enable debug display.
 */
UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DDKNOCKOFFTESTS_API UFunctionalTestDebugComponent
    : public UActorComponent, public IDebugInformationProvider {
    GENERATED_BODY()

public:
    UFunctionalTestDebugComponent();

    // IDebugInformationProvider implementation
    virtual FString GetDebugInformation() const override;
    virtual FString GetDebugCategory() const override;

    // Public interface for tests
    UFUNCTION(BlueprintCallable, Category = "Debug")
    // ReSharper disable once CppUEBlueprintCallableFunctionUnused
    void UpdatePhaseInfo(const FString& PhaseName, int32 PhaseIndex);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    // ReSharper disable once CppUEBlueprintCallableFunctionUnused
    void ManualRegister();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    // ReSharper disable once CppUEBlueprintCallableFunctionUnused
    void ManualUnregister();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bEnableDebugDisplay = true;

    float TestStartTime = 0.0f;
    FString CurrentPhaseInfo;

    AFunctionalTest* GetOwningFunctionalTest() const;
    UDebugInformationManager* GetDebugManager() const;
};
