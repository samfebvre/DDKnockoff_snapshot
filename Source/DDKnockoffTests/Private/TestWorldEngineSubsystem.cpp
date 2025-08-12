#include "TestWorldEngineSubsystem.h"
#include "EngineUtils.h"
#include "TestWorldGameInstance.h"

void UTestWorldEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection) {
    Super::Initialize(Collection);
}

void UTestWorldEngineSubsystem::Deinitialize() {
    // Dispose of the created Worlds
    if (!PrivateWorldDatas.IsEmpty()) {
        for (const auto& [Name, Env] : PrivateWorldDatas) {
            // This shouldn't happen, might want to log a warning
            Env.World->DestroyWorld(true);
            Env.GameInstance->RemoveFromRoot();
        }
        PrivateWorldDatas.Empty();
    }

    if (SharedWorldData.World) {
        SharedWorldData.World->DestroyWorld(true);
        SharedWorldData.GameInstance->RemoveFromRoot();
    }

    Super::Deinitialize();
}

FTestWorldHelper UTestWorldEngineSubsystem::GetPrivateWorldHelper() {
    static uint32 WorldCounter = 0;
    const FName WorldName = *FString::Printf(
        TEXT("UTestWorldEngineSubsystem_PrivateWorld_%d"),
        WorldCounter++);

    check(IsInGameThread());

    const auto& [GameInstance, World] = PrivateWorldDatas.Add(
        WorldName,
        MakeTestWorldData(WorldName));
    return FTestWorldHelper{this, World, false};
}

FTestWorldHelper UTestWorldEngineSubsystem::GetSharedWorldHelper() {
    // Lazy initialize the shared World,
    // because doing so in UEngineSubsystem::Initialize
    // is too early!
    if (!SharedWorldData.World) { SharedWorldData = MakeTestWorldData("TestWorld_SharedWorld"); }

    FTestWorldHelper Helper{this, SharedWorldData.World, true};
    return Helper;
}

void UTestWorldEngineSubsystem::DestroyPrivateWorldData(FName Name) {
    const auto& [GameInstance, World] = PrivateWorldDatas.FindAndRemoveChecked(Name);
    World->EndPlay(EEndPlayReason::Destroyed);

    // Make sure to cleanup all actors immediately
    // DestroyWorld doesn't do this and instead waits for GC to clear everything up
    for (auto It = TActorIterator<AActor>(World); It; ++It) { It->Destroy(); }

    GEngine->DestroyWorldContext(World);
    World->DestroyWorld(true);
    GameInstance->Shutdown();
    GameInstance->RemoveFromRoot();
}

FTestWorldData UTestWorldEngineSubsystem::MakeTestWorldData(FName Name) {
    check(IsInGameThread());

    // Create a Game World
    FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, Name, GetTransientPackage());
    UTestWorldGameInstance* GameInstance = NewObject<UTestWorldGameInstance>();
    GameInstance->AddToRoot(); // Avoids GC

    WorldContext.SetCurrentWorld(World);
    World->UpdateWorldComponents(true, true);
    World->AddToRoot();
    World->SetFlags(RF_Public | RF_Standalone);
    // Engine shouldn't Tick this UWorld.
    World->SetShouldTick(false);

    // Remember UTestWorldGameInstance::InitForTest?
    GameInstance->InitForTest(*World);

#if WITH_EDITOR
    GEngine->BroadcastLevelActorListChanged();
#endif

    World->InitializeActorsForPlay(FURL());
    auto* Settings = World->GetWorldSettings();
    // Unreal clamps the DeltaTime
    Settings->MinUndilatedFrameTime = 0.0001;
    Settings->MaxUndilatedFrameTime = 10;

    // Finally, we start playing
    World->BeginPlay();

    return {GameInstance, World};
}
