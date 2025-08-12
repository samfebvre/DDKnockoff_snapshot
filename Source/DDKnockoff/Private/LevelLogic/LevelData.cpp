#include "LevelLogic/LevelData.h"

void ULevelData::ValidateConfiguration() const {
    // Validate that there is at least one wave
    ensureAlwaysMsgf(!Waves.IsEmpty(),
                     TEXT("LevelData: Waves array is empty! Level will not function correctly"));

    // Validate each wave
    for (int32 WaveIndex = 0; WaveIndex < Waves.Num(); WaveIndex++) {
        const FWaveData& Wave = Waves[WaveIndex];
        ensureAlwaysMsgf(Wave.IsValid(), TEXT("LevelData: Wave[%d] is invalid!"), WaveIndex);

        // Additional validation: Check each spawner entry
        for (int32 SpawnerIndex = 0; SpawnerIndex < Wave.SpawnerEntries.Num(); SpawnerIndex++) {
            const FSpawnerWaveData& SpawnerData = Wave.SpawnerEntries[SpawnerIndex];
            ensureAlwaysMsgf(SpawnerData.SpawnerID >= 0,
                             TEXT(
                                 "LevelData: Wave[%d].SpawnerEntries[%d] has invalid SpawnerID (%d)"
                             ),
                             WaveIndex,
                             SpawnerIndex,
                             SpawnerData.SpawnerID);

            // Check each enemy in the spawner entry
            for (int32 EnemyIndex = 0; EnemyIndex < SpawnerData.Enemies.Num(); EnemyIndex++) {
                const FEnemySpawnData& EnemyData = SpawnerData.Enemies[EnemyIndex];
                ensureAlwaysMsgf(EnemyData.EnemyClass != nullptr,
                                 TEXT(
                                     "LevelData: Wave[%d].SpawnerEntries[%d].Enemies[%d] has null EnemyClass"
                                 ),
                                 WaveIndex,
                                 SpawnerIndex,
                                 EnemyIndex);
                ensureAlwaysMsgf(EnemyData.Count > 0,
                                 TEXT(
                                     "LevelData: Wave[%d].SpawnerEntries[%d].Enemies[%d] has invalid Count (%d)"
                                 ),
                                 WaveIndex,
                                 SpawnerIndex,
                                 EnemyIndex,
                                 EnemyData.Count);
            }
        }
    }
}
