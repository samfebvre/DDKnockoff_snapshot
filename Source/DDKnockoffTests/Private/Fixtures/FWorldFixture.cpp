#include "Fixtures/FWorldFixture.h"

#include "EngineUtils.h"

FWorldFixture::FWorldFixture(const FURL& URL) {
    if (GEngine != nullptr) {
        static uint32 WorldCounter = 0;
        const FString WorldName = FString::Printf(TEXT("WorldFixture_%d"), WorldCounter++);

        if (UWorld* World = UWorld::CreateWorld(EWorldType::Game,
                                                false,
                                                *WorldName,
                                                GetTransientPackage())) {
            FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
            WorldContext.SetCurrentWorld(World);

            World->InitializeActorsForPlay(URL);
            if (IsValid(World->GetWorldSettings())) {
                // Need to do this manually since world doesn't have a game mode
                World->GetWorldSettings()->NotifyBeginPlay();
                World->GetWorldSettings()->NotifyMatchStarted();
            }
            World->BeginPlay();

            WeakWorld = MakeWeakObjectPtr(World);
        }
    }
}

UWorld* FWorldFixture::GetWorld() const { return WeakWorld.Get(); }

FWorldFixture::~FWorldFixture() {
    UWorld* World = WeakWorld.Get();
    if (World != nullptr && GEngine != nullptr) {
        World->EndPlay(EEndPlayReason::Destroyed);

        // Make sure to cleanup all actors immediately
        // DestroyWorld doesn't do this and instead waits for GC to clear everything up
        for (auto It = TActorIterator<AActor>(World); It; ++It) { It->Destroy(); }

        GEngine->DestroyWorldContext(World);
        World->DestroyWorld(false);
    }
}
