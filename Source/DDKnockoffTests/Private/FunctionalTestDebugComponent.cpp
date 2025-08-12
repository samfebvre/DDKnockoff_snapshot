#include "FunctionalTestDebugComponent.h"
#include "Debug/DebugInformationManager.h"
#include "FunctionalTest.h"
#include "Engine/World.h"
#include "Core/ManagerHandlerSubsystem.h"

UFunctionalTestDebugComponent::UFunctionalTestDebugComponent() {
    PrimaryComponentTick.bCanEverTick = false;
}

FString UFunctionalTestDebugComponent::GetDebugInformation() const {
    FString DebugInfo;

    if (const AFunctionalTest* FunctionalTest = GetOwningFunctionalTest()) {
        DebugInfo.Append(FString::Printf(TEXT("Test: %s\n"), *FunctionalTest->GetName()));

        const FString StateText = FunctionalTest->IsRunning()
                                      ? TEXT("Running")
                                      : TEXT("Not Running");
        DebugInfo.Append(FString::Printf(TEXT("State: %s\n"), *StateText));

        if (FunctionalTest->IsRunning() && TestStartTime > 0.0f) {
            const float ElapsedTime = GetWorld()->GetTimeSeconds() - TestStartTime;
            DebugInfo.Append(FString::Printf(TEXT("Elapsed: %.2fs\n"), ElapsedTime));
        }
    }

    if (!CurrentPhaseInfo.IsEmpty()) {
        DebugInfo.Append(FString::Printf(TEXT("Phase: %s"), *CurrentPhaseInfo));
    }

    return DebugInfo;
}

FString UFunctionalTestDebugComponent::GetDebugCategory() const { return TEXT("Test Status"); }

void UFunctionalTestDebugComponent::UpdatePhaseInfo(const FString& PhaseName, int32 PhaseIndex) {
    CurrentPhaseInfo = FString::Printf(TEXT("%s (Index: %d)"), *PhaseName, PhaseIndex);

    // Record test start time on first phase
    if (PhaseIndex == 0 && TestStartTime == 0.0f) { TestStartTime = GetWorld()->GetTimeSeconds(); }
}

AFunctionalTest* UFunctionalTestDebugComponent::GetOwningFunctionalTest() const {
    return Cast<AFunctionalTest>(GetOwner());
}

UDebugInformationManager* UFunctionalTestDebugComponent::GetDebugManager() const {
    if (UWorld* World = GetWorld()) {
        return UManagerHandlerSubsystem::GetManager<UDebugInformationManager>(World);
    }
    return nullptr;
}

void UFunctionalTestDebugComponent::ManualRegister() {
    if (!bEnableDebugDisplay) { return; }

    UDebugInformationManager* DebugSubsystem = GetDebugManager();
    if (DebugSubsystem) {
        // Register ourselves as a debug provider
        TScriptInterface<IDebugInformationProvider> Provider;
        Provider.SetObject(this);
        Provider.SetInterface(Cast<IDebugInformationProvider>(this));
        DebugSubsystem->RegisterDebugInformationProvider(Provider);

        UE_LOG(LogTemp,
               Log,
               TEXT(
                   "FunctionalTestDebugComponent manually registered with DebugInformationSubsystem"
               ));
    } else {
        UE_LOG(LogTemp,
               Warning,
               TEXT("DebugInformationSubsystem not available for functional test"));
    }
}

void UFunctionalTestDebugComponent::ManualUnregister() {
    if (!bEnableDebugDisplay) { return; }

    UDebugInformationManager* DebugManager = GetDebugManager();
    if (DebugManager) {
        // Unregister ourselves
        TScriptInterface<IDebugInformationProvider> Provider;
        Provider.SetObject(this);
        Provider.SetInterface(Cast<IDebugInformationProvider>(this));
        DebugManager->UnregisterDebugInformationProvider(Provider);

        UE_LOG(LogTemp,
               Log,
               TEXT(
                   "FunctionalTestDebugComponent manually unregistered from DebugInformationSubsystem"
               ));
    }
}
