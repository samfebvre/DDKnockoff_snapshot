#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WaveData.generated.h"

/**
 * Data structure defining enemy spawn configuration for a single spawner.
 * Specifies which enemy types and quantities to spawn.
 */
USTRUCT(BlueprintType)
struct FEnemySpawnData {
    GENERATED_BODY()

    /** Enemy class to spawn (must implement Entity interface) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=( MustImplement = "Entity" ))
    TSubclassOf<AActor> EnemyClass;

    /** Number of enemies to spawn */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count;

    /**
     * Validate the spawn data configuration.
     * @return true if spawn data is valid
     */
    bool IsValid() const { return EnemyClass != nullptr && Count > 0; }
};

/**
 * Data structure defining enemy spawn configuration for a specific spawner in a wave.
 * Links a spawner ID to its enemy spawn queue for that wave.
 */
USTRUCT(BlueprintType)
struct FSpawnerWaveData {
    GENERATED_BODY()

    /** ID of the spawner that should spawn these enemies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SpawnerID;

    /** Array of enemy types and counts to spawn from this spawner */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FEnemySpawnData> Enemies;

    /**
     * Validate the spawner wave data configuration.
     * @return true if all enemy data is valid and spawner ID is valid
     */
    bool IsValid() const {
        if (SpawnerID < 0 || Enemies.IsEmpty()) { return false; }

        for (const FEnemySpawnData& EnemyData : Enemies) {
            if (!EnemyData.IsValid()) { return false; }
        }

        return true;
    }
};

/**
 * Data structure defining the complete configuration for a single wave.
 * Contains spawn data for all spawners participating in this wave.
 */
USTRUCT(BlueprintType)
struct FWaveData {
    GENERATED_BODY()

    /** Spawn configuration for each spawner in this wave */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FSpawnerWaveData> SpawnerEntries;

    /**
     * Validate the complete wave data configuration.
     * @return true if all spawner entries are valid
     */
    bool IsValid() const {
        if (SpawnerEntries.IsEmpty()) { return false; }

        for (const FSpawnerWaveData& SpawnerData : SpawnerEntries) {
            if (!SpawnerData.IsValid()) { return false; }
        }

        return true;
    }
};
