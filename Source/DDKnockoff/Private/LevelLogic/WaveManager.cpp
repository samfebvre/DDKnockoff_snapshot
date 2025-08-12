#include "LevelLogic/WaveManager.h"

#include "EngineUtils.h"
#include "LevelLogic/Spawner.h"
#include "LevelLogic/LevelData.h"
#include "LevelLogic/WaveData.h"
#include "LevelLogic/WaveManagerSettings.h"
#include "Debug/DebugInformationManager.h"
#include "Core/DDKnockoffGameSettings.h"
#include "Core/ManagerHandlerSubsystem.h"

class ADDKnockoffGameMode;

UWaveManager::UWaveManager() {
    CurrentState = EWaveState::Uninitialized;
    CountdownStartTime = 0.0;
    CountdownEndTime = 0.0;
    NextSpawnTime = 0.0;
    CurrentWaveIndex = 0;
}

void UWaveManager::Deinitialize() {
    // Unregister as a debug information provider
    if (UDebugInformationManager* DebugSubsystem = UManagerHandlerSubsystem::GetManager<
        UDebugInformationManager>(GetWorld())) {
        DebugSubsystem->UnregisterDebugInformationProvider(this);
    }
}

void UWaveManager::Initialize() {
    // Load settings from data asset
    LoadSettings();

    // TODO - this is going to cause an issue if the debug information manager is not initialized yet. We would need to create a dependency system to handle this most likely.
    // Unregister as a debug information provider
    if (UDebugInformationManager* DebugSubsystem = UManagerHandlerSubsystem::GetManager<
        UDebugInformationManager>(GetWorld())) {
        DebugSubsystem->RegisterDebugInformationProvider(this);
    }

    // Ensure settings are valid
    if (!Settings) {
        UE_LOG(LogTemp,
               Error,
               TEXT("WaveManager: Failed to load settings. Wave system will not function."));
    } else { UE_LOG(LogTemp, Log, TEXT("WaveManager initialized with settings")); }

    CollectSpawners();
}

void UWaveManager::LoadSettings() {
    // Load settings path from game settings
    const UDDKnockoffGameSettings* GameSettings = UDDKnockoffGameSettings::Get();
    if (GameSettings && !GameSettings->WaveManagerSettingsAsset.IsNull()) {
        SettingsAsset = TSoftObjectPtr<
            UWaveManagerSettings>(GameSettings->WaveManagerSettingsAsset);

        if (SettingsAsset.IsValid() || SettingsAsset.IsPending()) {
            Settings = SettingsAsset.LoadSynchronous();
            if (Settings) {
                Settings->ValidateConfiguration();
                UE_LOG(LogTemp,
                       Log,
                       TEXT("WaveManager: Loaded settings from %s"),
                       *GameSettings->WaveManagerSettingsAsset.ToString());

                // Cache wave data after loading settings
                CacheWaveData();
            } else {
                UE_LOG(LogTemp,
                       Error,
                       TEXT("WaveManager: Failed to load settings from %s"),
                       *GameSettings->WaveManagerSettingsAsset.ToString());
            }
        } else {
            UE_LOG(LogTemp,
                   Error,
                   TEXT("WaveManager: Settings asset path is invalid: %s"),
                   *GameSettings->WaveManagerSettingsAsset.ToString());
        }
    } else {
        UE_LOG(LogTemp,
               Error,
               TEXT("WaveManager: No wave manager settings asset configured in game settings"));
    }
}

void UWaveManager::CacheWaveData() {
    WaveCache.Empty();
    if (!Settings || !Settings->LevelData) { return; }

    // Pre-calculate data for each wave
    for (const FWaveData& Wave : Settings->LevelData->Waves) {
        FWaveSpawnCache& WaveSpawnCache = WaveCache.AddDefaulted_GetRef();

        // Process each spawner's data
        for (const FSpawnerWaveData& SpawnerData : Wave.SpawnerEntries) {
            WaveSpawnCache.ActiveSpawnerIds.Add(SpawnerData.SpawnerID);

            // Create queue for this spawner

            TArray<TSubclassOf<AActor>>& SpawnQueue = WaveSpawnCache.SpawnerQueues.Emplace(
                SpawnerData.SpawnerID);

            // Fill the queue with enemies in order
            for (const FEnemySpawnData& EnemyData : SpawnerData.Enemies) {
                // Add the enemy type to the queue Count times
                for (int32 i = 0; i < EnemyData.Count; ++i) {
                    SpawnQueue.Add(EnemyData.EnemyClass);
                }
            }
        }
    }
}

void UWaveManager::InitializeActiveSpawners() {
    CurrentActiveSpawners.Empty();
    if (WaveCache.IsValidIndex(CurrentWaveIndex)) {
        CurrentActiveSpawners = WaveCache[CurrentWaveIndex].ActiveSpawnerIds;
    }
}

void UWaveManager::StartNextWave() {
    if (CurrentState != EWaveState::WaitingToStart) { return; }

    if (!WaveCache.IsValidIndex(CurrentWaveIndex)) {
        UE_LOG(LogTemp, Warning, TEXT("No more waves available or invalid level data!"));
        SetWaveState(EWaveState::SpawningComplete);
        return;
    }

    // Initialize wave data
    InitializeActiveSpawners();

    // Set countdown timestamps
    CountdownStartTime = FPlatformTime::Seconds();
    CountdownEndTime = CountdownStartTime + Settings->WaveCountdownDuration;
    SetWaveState(EWaveState::Countdown);
}

float UWaveManager::GetCountdownTime() const {
    if (CurrentState != EWaveState::Countdown) { return 0.0f; }

    const double CurrentTime = FPlatformTime::Seconds();
    const double RemainingTime = CountdownEndTime - CurrentTime;
    return FMath::Max(0.0f, static_cast<float>(RemainingTime));
}

void UWaveManager::Tick(float DeltaTime) {
    if (CurrentState == EWaveState::Countdown) {
        const double CurrentTime = FPlatformTime::Seconds();
        if (CurrentTime >= CountdownEndTime) {
            NextSpawnTime = CurrentTime;
            SetWaveState(EWaveState::Spawning);
        }
    } else if (CurrentState == EWaveState::Spawning) { ProcessSpawning(); }
}

FString UWaveManager::GetDebugCategory() const { return TEXT("Wave System"); }

FString UWaveManager::GetDebugInformation() const {
    // Get state name
    FString StateString;
    if (const UEnum* EnumPtr = StaticEnum<EWaveState>()) {
        StateString = EnumPtr->GetNameStringByValue(static_cast<int32>(CurrentState));
    } else { StateString = TEXT("Unknown"); }

    // Number of enemies remaining in current wave
    int32 EnemiesRemaining = 0;
    if (Settings->LevelData && Settings->LevelData->Waves.IsValidIndex(CurrentWaveIndex)) {
        for (const FWaveData& CurrentWave = Settings->LevelData->Waves[CurrentWaveIndex]; const
             FSpawnerWaveData& SpawnerData : CurrentWave.SpawnerEntries) {
            for (const FEnemySpawnData& EnemyData : SpawnerData.Enemies) {
                EnemiesRemaining += EnemyData.Count;
            }
        }
    }

    // Number of enemies remaining string
    const FString EnemiesRemainingString = FString::Printf(TEXT("%d"), EnemiesRemaining);

    // Calculate time remaining based on current state
    FString TimeRemainingString;
    const double CurrentTime = FPlatformTime::Seconds();

    switch (CurrentState) {
        case EWaveState::Countdown:
            TimeRemainingString = FString::Printf(
                TEXT("%.2f"),
                FMath::Max(0.0, CountdownEndTime - CurrentTime));
            break;
        default:
            TimeRemainingString = TEXT("N/A");
            break;
    }

    return FString::Printf(
        TEXT("State: %s\nWave: %d\nEnemies Remaining: %s\nTime Remaining: %s"),
        *StateString,
        CurrentWaveIndex + 1,
        *EnemiesRemainingString,
        *TimeRemainingString
        );
}


void UWaveManager::ProcessSpawning() {
    const double CurrentTime = FPlatformTime::Seconds();
    if (CurrentTime < NextSpawnTime) { return; }

    if (SpawnWaveEnemies()) { NextSpawnTime = CurrentTime + Settings->TimeBetweenSpawns; } else {
        // No more enemies to spawn in this wave
        CurrentWaveIndex++;
        SetWaveState(EWaveState::WaitingToStart);

        // Check if we've completed all waves
        if (!Settings->LevelData || !Settings->LevelData->Waves.IsValidIndex(CurrentWaveIndex)) {
            SetWaveState(EWaveState::SpawningComplete);
        }
    }
}


bool UWaveManager::CanSpawnEnemies() const {
    return Settings->LevelData &&
           Settings->LevelData->Waves.IsValidIndex(CurrentWaveIndex) &&
           !CurrentActiveSpawners.IsEmpty();
}

bool UWaveManager::TrySpawnEnemyFromSpawner(int32 SpawnerId, bool& OutSpawnerStillActive) {
    OutSpawnerStillActive = false;

    const ASpawner* Spawner = FindSpawnerById(SpawnerId);
    if (!Spawner) { return false; }

    // Get the spawn queue for this spawner
    if (TArray<TSubclassOf<AActor>>* SpawnQueuePtr = WaveCache[CurrentWaveIndex].SpawnerQueues.
        Find(SpawnerId)) {
        auto& SpawnQueue = *SpawnQueuePtr;
        // Assume not empty based on our algorithm
        const TSubclassOf<AActor> NextEnemyClass = SpawnQueue[0];
        SpawnQueue.RemoveAt(0); // Remove the enemy class from the queue

        // Spawn the next enemy type from the queue
        Spawner->SpawnActor(NextEnemyClass);

        // Check if there are more enemies in the queue
        OutSpawnerStillActive = !SpawnQueue.IsEmpty();
        return true;
    }

    return false;
}

void UWaveManager::RemoveInactiveSpawners(const TArray<int32>& SpawnersToRemove) {
    for (const int32 SpawnerId : SpawnersToRemove) { CurrentActiveSpawners.Remove(SpawnerId); }
}

bool UWaveManager::SpawnWaveEnemies() {
    if (!CanSpawnEnemies()) { return false; }

    bool AnythingSpawned = false;
    TArray<int32> SpawnersToRemove;

    // Try to spawn from each active spawner
    for (const int32 SpawnerId : CurrentActiveSpawners) {
        bool SpawnerStillActive = false;
        if (TrySpawnEnemyFromSpawner(SpawnerId, SpawnerStillActive)) { AnythingSpawned = true; }

        if (!SpawnerStillActive) { SpawnersToRemove.Add(SpawnerId); }
    }

    RemoveInactiveSpawners(SpawnersToRemove);
    return AnythingSpawned;
}

ASpawner* UWaveManager::FindSpawnerById(const int32 SpawnerId) const {
    const TWeakObjectPtr<ASpawner>* FoundSpawner = SpawnerMap.Find(SpawnerId);
    return FoundSpawner && FoundSpawner->IsValid() ? FoundSpawner->Get() : nullptr;
}

void UWaveManager::SetWaveState(EWaveState NewState) {
    CurrentState = NewState;

    // Log state changes for debugging
    if (const UEnum* EnumPtr = StaticEnum<EWaveState>()) {
        UE_LOG(LogTemp,
               Log,
               TEXT("Wave State Changed to: %s"),
               *EnumPtr->GetNameStringByValue(static_cast<int64>(NewState)));
    }

    switch (NewState) {
        case EWaveState::Spawning:
            UE_LOG(LogTemp, Log, TEXT("Wave spawning started!"));

        // Open doors
            OpenDoorsForActiveSpawners();
            break;

        default:
            break;
    }
}

void UWaveManager::CollectSpawners() {
    // Clear existing spawners
    SpawnerMap.Empty();

    // Search the world for all actors of type ASpawner
    for (TActorIterator<ASpawner> It(GetWorld()); It; ++It) {
        if (ASpawner* Spawner = *It) {
            SpawnerMap.Add(Spawner->SpawnerID, Spawner);
            UE_LOG(LogTemp, Log, TEXT("Collected Spawner with ID: %d"), Spawner->SpawnerID);
        }
    }

    // If we have spawners, transition to WaitingToStart state
    if (SpawnerMap.Num() > 0) { SetWaveState(EWaveState::WaitingToStart); }
}

void UWaveManager::CloseDoors() {
    // Close the door for every spawner in the spawner map
    for (const auto KVPair : SpawnerMap) { KVPair.Value->CloseDoors(); }
}

void UWaveManager::OpenDoorsForActiveSpawners() {
    // Open all the doors for the active spawners
    for (const int32 SpawnerId : CurrentActiveSpawners) {
        if (const ASpawner* Spawner = FindSpawnerById(SpawnerId)) { Spawner->OpenDoors(); }
    }
}
