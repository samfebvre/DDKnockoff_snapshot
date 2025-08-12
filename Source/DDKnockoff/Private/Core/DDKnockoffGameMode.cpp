#include "Core/DDKnockoffGameMode.h"

#include "Entities//EntityManager.h"
#include "LevelLogic/WaveManager.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"
#include "Debug/DebugInformationManager.h"
#include "Kismet/GameplayStatics.h"
#include "Currency/CurrencyManager.h"
#include "Currency/CurrencyManagerSettings.h"
#include "ResourceChest/ResourceChest.h"
#include "Core/DDKnockoffGameSettings.h"
#include "Core/ManagerHandlerSubsystem.h"
#include "Structures/StructurePlacementManager.h"

ADDKnockoffGameMode::ADDKnockoffGameMode()
    : WaveManager(nullptr), EntityManager(nullptr), ReadyUpProgress(0.0f), bIsReadyingUp(false) {
    PrimaryActorTick.bCanEverTick = true;
    
    CurrentGamePhase = EGamePhase::InitialPreparation;

    // Load player character class from settings
    const UDDKnockoffGameSettings* GameSettings = UDDKnockoffGameSettings::Get();
    if (GameSettings && !GameSettings->DefaultPlayerCharacterClass.IsEmpty()) {
        static ConstructorHelpers::FClassFinder<APawn> PlayerCharacterClassFinder(
            *GameSettings->DefaultPlayerCharacterClass);
        DefaultPawnClass = PlayerCharacterClassFinder.Class;
    }
}

void ADDKnockoffGameMode::Tick(float DeltaSeconds) {
    Super::Tick(DeltaSeconds);

    // TODO - handle ready up input in a better way, this is a very temporary solution.
    // Update ready-up progress if player is holding the button
    if (bIsReadyingUp && IsInAPreparationPhase()) { UpdateReadyUpProgress(DeltaSeconds); }
}

void ADDKnockoffGameMode::OnCharacterPressedReadyUpInput() {
    if (!IsInAPreparationPhase()) { return; }
    if (!bIsReadyingUp) { StartReadyUp(); }
}

void ADDKnockoffGameMode::OnCharacterReleasedReadyUpInput() {
    if (!IsInAPreparationPhase()) { return; }
    if (bIsReadyingUp) { CancelReadyUp(); }
}


FString ADDKnockoffGameMode::GetDebugInformation() const {
    // Get the current phase name
    const FString PhaseString = StaticEnum<EGamePhase>()->GetNameStringByValue(
        static_cast<int64>(CurrentGamePhase));

    // Create a ready up status string
    FString ReadyUpStatus = bIsReadyingUp ? TEXT("Ready-up in progress") : TEXT("Not readying up");
    if (bIsReadyingUp) {
        ReadyUpStatus.Append(
            FString::Printf(TEXT(" (Progress: %.2f)"), ReadyUpProgress / RequiredReadyUpTime));
    }

    // Get time remaining for timer-based phases
    float RemainingTime = 0.0f;
    bool HasTimer = false;

    if (CurrentGamePhase == EGamePhase::Defeat ||
        CurrentGamePhase == EGamePhase::Reward) {
        if (GetWorldTimerManager().IsTimerActive(PhaseTransitionTimerHandle)) {
            RemainingTime = GetWorldTimerManager().GetTimerRemaining(PhaseTransitionTimerHandle);
            HasTimer = true;
        }
    }

    const FString TimeRemainingString = HasTimer
                                            ? FString::Printf(TEXT("%.2f"), RemainingTime)
                                            : TEXT("N/A");

    return FString::Printf(TEXT("Phase: %s\nReady-Up Status: %s\nPhase time Remaining: %s"),
                           *PhaseString,
                           *ReadyUpStatus,
                           *TimeRemainingString);
}


FString ADDKnockoffGameMode::GetDebugCategory() const { return TEXT("Game Flow"); }

void ADDKnockoffGameMode::BeginPlay() { Super::BeginPlay(); }

void ADDKnockoffGameMode::StartPlay() {
    ManagerHandlerSubsystem = GetWorld()->GetSubsystem<UManagerHandlerSubsystem>();
    ensureAlways(ManagerHandlerSubsystem);

    ManagerHandlerSubsystem->SetManagers({
        UDebugInformationManager::StaticClass(),
        UEntityManager::StaticClass(),
        UWaveManager::StaticClass(),
        UCurrencyManager::StaticClass(),
        UStructurePlacementManager::StaticClass(),
    });

    // TODO - maybe make these references, these subsystems should be available for the game modes whole lifetime.
    EntityManager = ManagerHandlerSubsystem->GetManager<UEntityManager>();
    WaveManager = ManagerHandlerSubsystem->GetManager<UWaveManager>();

    // TODO - Currency manager settings loading should be done within the class itself, not here, like the other managers.
    CurrencyManager = ManagerHandlerSubsystem->GetManager<UCurrencyManager>();
    if (CurrencyManagerSettings.IsValid() || CurrencyManagerSettings.IsPending()) {
        UCurrencyManagerSettings* LoadedSettings = CurrencyManagerSettings.LoadSynchronous();
        CurrencyManager->SetSettings(LoadedSettings);
    }
    
    BindToEntityEvents();
    
    StartInitialPreparationPhase();

    // Register with debug information subsystem
    if (UDebugInformationManager* DebugSubsystem = UManagerHandlerSubsystem::GetManager<
        UDebugInformationManager>(GetWorld())) {
        DebugSubsystem->RegisterDebugInformationProvider(this);
    }

    Super::StartPlay();
}

void ADDKnockoffGameMode::BindToEntityEvents() {
    EntityManager->OnEntityRemoved.RemoveDynamic(this, &ADDKnockoffGameMode::OnEntityRemoved);
    EntityManager->OnEntityRemoved.AddDynamic(this, &ADDKnockoffGameMode::OnEntityRemoved);
}

bool ADDKnockoffGameMode::AreAllEnemiesDead() const {
    if (!EntityManager) { return true; }

    // Get all entities that are characters and enemies
    const TArray<TScriptInterface<IEntity>> EnemyEntities = EntityManager->
        GetEntitiesByFactionAndType(EFaction::Enemy, EEntityType::Character);
    return EnemyEntities.Num() == 0;
}

bool ADDKnockoffGameMode::AreAllCrystalsDead() const {
    if (!EntityManager) { return true; }

    // Get all entities that are crystals
    const TArray<TScriptInterface<IEntity>> CrystalEntities = EntityManager->
        GetEntitiesByFactionAndType(EFaction::Player, EEntityType::Structure_Crystal);
    return CrystalEntities.Num() == 0;
}

void ADDKnockoffGameMode::OnEntityRemoved(const TScriptInterface<IEntity>& Entity) {
    // If we're in combat phase and an enemy was removed, check if all enemies are dead
    if (CurrentGamePhase == EGamePhase::Combat && Entity) {
        if (Entity->GetFaction() == EFaction::Enemy) {
            if (AreAllEnemiesDead()) {
                // If this was the last wave, go to reward phase
                if (WaveManager && WaveManager->GetCurrentState() == EWaveState::SpawningComplete) {
                    StartRewardPhase();
                } else { StartInterCombatPreparationPhase(); }
            }
        }
        // Check for crystal deaths
        else if (Entity->GetEntityType() == EEntityType::Structure_Crystal) {
            if (AreAllCrystalsDead()) { StartDefeatPhase(); }
        }
    }
}

void ADDKnockoffGameMode::SetGamePhase(EGamePhase NewPhase) {
    if (CurrentGamePhase == NewPhase) { return; }

    CurrentGamePhase = NewPhase;

    // Log phase changes for debugging
    if (const UEnum* EnumPtr = StaticEnum<EGamePhase>()) {
        UE_LOG(LogTemp,
               Log,
               TEXT("Game Phase Changed to: %s"),
               *EnumPtr->GetNameStringByValue(static_cast<int64>(NewPhase)));
    }
}

void ADDKnockoffGameMode::StartCombatPhase() {
    SetGamePhase(EGamePhase::Combat);

    // Clear any existing timers
    GetWorldTimerManager().ClearTimer(PhaseTransitionTimerHandle);

    // If wave manager is waiting to start, begin the wave
    if (WaveManager && WaveManager->GetCurrentState() == EWaveState::WaitingToStart) {
        WaveManager->StartNextWave();
    }
}

void ADDKnockoffGameMode::StartInterCombatPreparationPhase() {
    // Only allow transition if all enemies are dead
    if (!AreAllEnemiesDead()) { return; }

    SetGamePhase(EGamePhase::InterCombatPreparation);
    WaveManager->CloseDoors();

    // Reset all chests in the level via the entity manager
    // TODO - I don't like this way of doing things, it's gross and wasteful. Need a better alternative than this.
    auto Entities = EntityManager->GetEntitiesByType(EEntityType::Interactable);
    for (const TScriptInterface<IEntity>& Entity : Entities) {
        if (Entity) {
            if (AResourceChest* Chest = Cast<AResourceChest>(Entity->GetActor())) {
                Chest->ResetChest();
            }
        }
    }
}

void ADDKnockoffGameMode::StartRewardPhase() {
    // Only allow transition if all enemies are dead
    if (!AreAllEnemiesDead()) { return; }

    SetGamePhase(EGamePhase::Reward);

    // Set timer to reload the map after reward duration
    GetWorldTimerManager().SetTimer(
        PhaseTransitionTimerHandle,
        this,
        &ADDKnockoffGameMode::ReloadCurrentMap,
        RewardPhaseDuration,
        false
        );
}

void ADDKnockoffGameMode::StartInitialPreparationPhase() {
    SetGamePhase(EGamePhase::InitialPreparation);
}

void ADDKnockoffGameMode::ReloadCurrentMap() const {
    // Get the current level name
    const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this);

    // Reload the current level
    UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
}

void ADDKnockoffGameMode::StartDefeatPhase() {
    SetGamePhase(EGamePhase::Defeat);

    // Set timer to reload the map after defeat duration
    GetWorldTimerManager().SetTimer(
        PhaseTransitionTimerHandle,
        this,
        &ADDKnockoffGameMode::ReloadCurrentMap,
        DefeatPhaseDuration,
        false
        );
}

void ADDKnockoffGameMode::StartReadyUp() {
    if (IsInAPreparationPhase() && !bIsReadyingUp) {
        bIsReadyingUp = true;

        UE_LOG(LogTemp, Log, TEXT("Ready-up started"))
    }
}

bool ADDKnockoffGameMode::IsInAPreparationPhase() const {
    return CurrentGamePhase == EGamePhase::InitialPreparation ||
           CurrentGamePhase == EGamePhase::InterCombatPreparation;
}

void ADDKnockoffGameMode::CancelReadyUp() {
    bIsReadyingUp = false;
    ReadyUpProgress = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("Ready-up cancelled"))
}

void ADDKnockoffGameMode::UpdateReadyUpProgress(float DeltaTime) {
    ReadyUpProgress += DeltaTime;

    // Check if we've held long enough
    if (ReadyUpProgress >= RequiredReadyUpTime) {
        // Reset progress and start combat
        ReadyUpProgress = 0.0f;
        bIsReadyingUp = false;
        StartCombatPhase();
    }
}
