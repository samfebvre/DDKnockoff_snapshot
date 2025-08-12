#include "TestWorldGameInstance.h"

void UTestWorldGameInstance::InitForTest(UWorld& World) {
    FWorldContext* TestWorldContext = GEngine->GetWorldContextFromWorld(&World);
    check(TestWorldContext);
    WorldContext = TestWorldContext;
    WorldContext->OwningGameInstance = this;
    World.SetGameInstance(this);
    World.SetGameMode(FURL()); // Now the UWorld::AuthorityGameMode will be valid

    Init();
}

TSubclassOf<AGameModeBase> UTestWorldGameInstance::OverrideGameModeClass(
    TSubclassOf<AGameModeBase> GameModeClass,
    const FString& MapName,
    const FString& Options,
    const FString& Portal) const { return ATestWorldGameMode::StaticClass(); }
