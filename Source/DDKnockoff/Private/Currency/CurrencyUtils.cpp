#include "Currency/CurrencyUtils.h"
#include "Currency/CurrencyManager.h"
#include "Currency/CurrencySpawner.h"
#include "Core/ManagerHandlerSubsystem.h"

bool UCurrencyUtils::SpawnCurrencyBurst(
    UWorld* World,
    UCurrencyManager* CurrencyManager,
    UCurrencySpawner* CurrencySpawner,
    int32 CurrencyAmount,
    int32 MinimumCrystalCount,
    const FVector& SpawnDirection,
    float LaunchSpeed) {
    // Validate inputs
    if (!World || !CurrencyManager || !CurrencySpawner || CurrencyAmount <= 0) { return false; }

    // Calculate what crystals to spawn based on amount and minimum count
    TArray<FCurrencySpawnInfo> SpawnInfos = CurrencyManager->CalculateCurrencySpawnInfo(
        CurrencyAmount,
        MinimumCrystalCount
        );

    // Spawn each type of crystal
    for (const FCurrencySpawnInfo& SpawnInfo : SpawnInfos) {
        if (SpawnInfo.CrystalClass && SpawnInfo.Count > 0) {
            CurrencySpawner->SpawnCurrencyBurst(
                SpawnInfo.CrystalClass,
                SpawnInfo.Count,
                SpawnDirection,
                LaunchSpeed
                );
        }
    }

    return true;
}

bool UCurrencyUtils::SpawnCurrencyBurstFromActor(
    AActor* SourceActor,
    UCurrencySpawner* CurrencySpawner,
    int32 CurrencyAmount,
    int32 MinimumCrystalCount,
    const FVector& SpawnDirection,
    float LaunchSpeed) {
    if (!SourceActor) { return false; }

    UWorld* World = SourceActor->GetWorld();
    if (!World) { return false; }

    // Get CurrencyManager from world
    UCurrencyManager* CurrencyManager = UManagerHandlerSubsystem::GetManager<
        UCurrencyManager>(World);
    if (!CurrencyManager) { return false; }

    return SpawnCurrencyBurst(World,
                              CurrencyManager,
                              CurrencySpawner,
                              CurrencyAmount,
                              MinimumCrystalCount,
                              SpawnDirection,
                              LaunchSpeed);
}
