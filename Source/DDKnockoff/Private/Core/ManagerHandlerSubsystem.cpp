#include "Core/ManagerHandlerSubsystem.h"

#include "Core/ManagerBase.h"


void UManagerHandlerSubsystem::SetManagers(const TArray<UClass*>& ManagerClasses) {
    ClearManagers();

    for (int i = 0; i < ManagerClasses.Num(); ++i) {
        UClass* ManagerClass = ManagerClasses[i];
        if (UManagerBase* NewManager = NewObject<UManagerBase>(this, ManagerClass)) {
            Managers.Add(ManagerClass, NewManager);
            NewManager->Initialize();
        } else {
            UE_LOG(LogTemp,
                   Warning,
                   TEXT("Failed to create manager of class %s"),
                   *ManagerClass->GetName());
        }
    }
}

void UManagerHandlerSubsystem::ClearManagers() {
    for (const auto& Pair : Managers) { Pair.Value->Deinitialize(); }
    Managers.Empty();
}

void UManagerHandlerSubsystem::RefreshManagers() {
    for (const auto& Pair : Managers) { Pair.Value->Deinitialize(); }

    for (const auto& Pair : Managers) { Pair.Value->Initialize(); }
}

void UManagerHandlerSubsystem::Deinitialize() { Super::Deinitialize(); }

void UManagerHandlerSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
    Super::Initialize(Collection);
}

void UManagerHandlerSubsystem::Tick(float DeltaTime) {
    for (const auto& Pair : Managers) { Pair.Value->Tick(DeltaTime); }
}

TStatId UManagerHandlerSubsystem::GetStatId() const {
    RETURN_QUICK_DECLARE_CYCLE_STAT(UManagerHandlerSubsystem, STATGROUP_Tickables);
}
