#pragma once

#include "CoreMinimal.h"
#include "Debug/DebugInformationProvider.h"
#include "UObject/Object.h"
#include "Containers/Queue.h"
#include "Core/ManagerBase.h"
#include "WaveManager.generated.h"

class UWaveManagerSettings;
struct FWaveData;
struct FSpawnerWaveData;
class ASpawner;
class ULevelData;

/**
 * Wave spawning state enumeration for tracking progression.
 */
UENUM()
enum class EWaveState : uint8 {
    Uninitialized,
    // Manager not yet initialized
    WaitingToStart,
    // Ready to begin wave countdown
    Countdown,
    // Counting down to wave start
    Spawning,
    // Actively spawning enemies
    SpawningComplete,
    // All enemies spawned, wave finished
    MAX UMETA(Hidden)
};

/**
 * Cache structure for optimized wave spawn data access.
 */
struct FWaveSpawnCache {
    /** Map of spawner IDs to their spawn queues for this wave */
    TMap<int32, TArray<TSubclassOf<AActor>>> SpawnerQueues;

    /** Set of spawner IDs that are active in this wave */
    TSet<int32> ActiveSpawnerIds;
};

/**
 * Manager for wave-based enemy spawning system with optimized caching and timing control.
 * Coordinates multiple spawners to create synchronized enemy waves with configurable timing.
 * Provides state tracking and debug information for wave progression monitoring.
 */
UCLASS()
class DDKNOCKOFF_API UWaveManager : public UManagerBase, public IDebugInformationProvider {
    GENERATED_BODY()

public:
    UWaveManager();

    // ManagerBase Interface
    virtual void Initialize() override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;

    // Wave control

    /**
     * Begin the next wave in the sequence with countdown and spawning.
     */
    void StartNextWave();

    /**
     * Discover and register all spawners in the level.
     */
    void CollectSpawners();

    /**
     * Close all doors.
     */
    void CloseDoors();

    /**
     * Open all doors associated with active spawners.
     */
    void OpenDoorsForActiveSpawners();

    // State queries

    /**
     * Get the current wave manager state.
     * @return Current wave state
     */
    UFUNCTION(BlueprintCallable, Category = "Wave Management")
    EWaveState GetCurrentState() const { return CurrentState; }

    /**
     * Get remaining time in current countdown phase.
     * @return Countdown time remaining in seconds
     */
    UFUNCTION(BlueprintCallable, Category = "Wave Management")
    float GetCountdownTime() const;

    // IDebugInformationProvider Interface Implementation
    virtual FString GetDebugCategory() const override;
    virtual FString GetDebugInformation() const override;

private:
    // Initialization

    /**
     * Load wave manager settings from configured data asset.
     */
    void LoadSettings();

    /**
     * Pre-calculate and cache wave spawn data for performance optimization.
     */
    void CacheWaveData();

    // State management

    /**
     * Transition to new wave state with proper cleanup and initialization.
     * @param NewState - State to transition to
     */
    void SetWaveState(EWaveState NewState);

    // Spawning system

    /**
     * Process ongoing wave spawning based on timing and spawner availability.
     */
    void ProcessSpawning();

    /**
     * Attempt to spawn enemies from all active spawners for current wave step.
     * @return true if spawning occurred
     */
    bool SpawnWaveEnemies();

    /**
     * Initialize the set of active spawners for the current wave.
     */
    void InitializeActiveSpawners();

    // Spawner management

    /**
     * Find spawner instance by ID for efficient lookup.
     * @param SpawnerId - ID of spawner to find
     * @return Spawner instance or null if not found
     */
    ASpawner* FindSpawnerById(int32 SpawnerId) const;

    // Validation helpers

    /**
     * Check if conditions are met for enemy spawning.
     * @return true if spawning can proceed
     */
    bool CanSpawnEnemies() const;

    /**
     * Attempt to spawn enemy from specific spawner.
     * @param SpawnerId - ID of spawner to use
     * @param OutSpawnerStillActive - Set to false if spawner queue is exhausted
     * @return true if spawn was successful
     */
    bool TrySpawnEnemyFromSpawner(int32 SpawnerId, bool& OutSpawnerStillActive);

    /**
     * Remove spawners that have finished their queues from active set.
     * @param SpawnersToRemove - Array of spawner IDs to deactivate
     */
    void RemoveInactiveSpawners(const TArray<int32>& SpawnersToRemove);

    // Configuration

    UPROPERTY(Transient)
    TObjectPtr<UWaveManagerSettings> Settings;

    TSoftObjectPtr<UWaveManagerSettings> SettingsAsset;

    // Runtime state

    UPROPERTY(Transient)
    EWaveState CurrentState;

    UPROPERTY(Transient)
    TSet<int32> CurrentActiveSpawners;

    // Cached data

    TMap<int32, TWeakObjectPtr<ASpawner>> SpawnerMap;
    TArray<FWaveSpawnCache> WaveCache;

    // Timing

    double CountdownStartTime;
    double CountdownEndTime;
    double NextSpawnTime;
    int32 CurrentWaveIndex;
};
