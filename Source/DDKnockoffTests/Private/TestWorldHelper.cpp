#include "TestWorldHelper.h"
#include "TestWorldEngineSubsystem.h"
#include "Engine/CoreSettings.h"
#include "HAL/ThreadManager.h"

FTestWorldHelper::FTestWorldHelper()
    : Subsystem(nullptr),
      World(nullptr),
      IsSharedWorld(false),
      OldGFrameCounter(GFrameCounter) {}

FTestWorldHelper::FTestWorldHelper(UTestWorldEngineSubsystem* InSubsystem,
                                   UWorld* InWorld,
                                   bool bInIsSharedWorld)
    : Subsystem(InSubsystem),
      World(InWorld),
      IsSharedWorld(bInIsSharedWorld),
      OldGFrameCounter(GFrameCounter) {}

FTestWorldHelper::FTestWorldHelper(FTestWorldHelper&& Other) noexcept
    : Subsystem(Other.Subsystem),
      World(Other.World),
      IsSharedWorld(Other.IsSharedWorld),
      OldGFrameCounter(Other.OldGFrameCounter) {
    Other.Subsystem = nullptr;
    Other.World = nullptr;
    Other.IsSharedWorld = true;
}

FTestWorldHelper& FTestWorldHelper::operator=(FTestWorldHelper&& Other) noexcept {
    if (this != &Other) {
        Cleanup(); // clean up current state if needed

        Subsystem = Other.Subsystem;
        World = Other.World;
        IsSharedWorld = Other.IsSharedWorld;
        OldGFrameCounter = Other.OldGFrameCounter;

        Other.Subsystem = nullptr;
        Other.World = nullptr;
        Other.IsSharedWorld = true;
    }
    return *this;
}

FTestWorldHelper::~FTestWorldHelper() { Cleanup(); }

void FTestWorldHelper::Cleanup() {
    if (!IsSharedWorld && IsValid(Subsystem) && IsValid(World)) {
        GFrameCounter = OldGFrameCounter;
        Subsystem->DestroyPrivateWorldData(World->GetFName());
    }

    World = nullptr;
    Subsystem = nullptr;
}

UWorld* FTestWorldHelper::GetWorld() const { return World; }


void FTestWorldHelper::Tick(float DeltaTime) const {
    check(IsInGameThread());
    check(World);

    StaticTick(DeltaTime, !!GAsyncLoadingUseFullTimeLimit, GAsyncLoadingTimeLimit / 1000.f);
    World->Tick(LEVELTICK_All, DeltaTime);
    FTickableGameObject::TickObjects(nullptr, LEVELTICK_All, false, DeltaTime);
    GFrameCounter++;
    FTaskGraphInterface::Get().ProcessThreadUntilIdle(ENamedThreads::GameThread);
    FThreadManager::Get().Tick();
    FTSTicker::GetCoreTicker().Tick(DeltaTime);
    GEngine->TickDeferredCommands();
}
