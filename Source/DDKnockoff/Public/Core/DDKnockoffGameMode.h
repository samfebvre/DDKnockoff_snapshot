#pragma once

#include "CoreMinimal.h"
#include "Debug/DebugInformationProvider.h"
#include "LevelLogic/WaveManager.h"
#include "GameFramework/GameModeBase.h"
#include "DDKnockoffGameMode.generated.h"

class UManagerHandlerSubsystem;
class UDebugInformationWidget;
class UDebugInformationManager;
class IEntity;
class UEntityManager;
class UWaveManager;
class ULevelData;
class UCurrencyManagerSettings;
class UCurrencyManager;

/**
 * Game phase enumeration for tracking the overall game flow state.
 */
UENUM(BlueprintType)
enum class EGamePhase : uint8 {
    InitialPreparation UMETA(DisplayName = "Initial Preparation"),
    // Setup phase before first wave
    Combat UMETA(DisplayName = "Combat"),
    // Active wave combat
    InterCombatPreparation UMETA(DisplayName = "Inter Combat Preparation"),
    // Between-wave preparation
    Reward UMETA(DisplayName = "Reward"),
    // Victory rewards phase
    Defeat UMETA(DisplayName = "Defeat") // Defeat state before restart
};

/**
 * Main game mode for DD Knockoff that manages game flow, wave progression, and manager coordination.
 * Handles phase transitions, ready-up mechanics, and entity lifecycle management.
 * Integrates with manager subsystem for centralized game state control.
 */
UCLASS(minimalapi)
class ADDKnockoffGameMode : public AGameModeBase, public IDebugInformationProvider {
    GENERATED_BODY()

public:
    ADDKnockoffGameMode();

    // Actor lifecycle
    virtual void Tick(float DeltaSeconds) override;

    // Input handling

    /**
     * Handle player pressing the ready-up input to advance phases.
     */
    void OnCharacterPressedReadyUpInput();

    /**
     * Handle player releasing the ready-up input to cancel advancement.
     */
    void OnCharacterReleasedReadyUpInput();

    // Phase management

    /**
     * Transition to combat phase and begin wave spawning.
     */
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void StartCombatPhase();

    /**
     * Transition to preparation phase between combat waves.
     */
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void StartInterCombatPreparationPhase();

    /**
     * Transition to reward phase after successful wave completion.
     */
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void StartRewardPhase();

    /**
     * Transition to initial preparation phase at game start.
     */
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void StartInitialPreparationPhase();

    /**
     * Transition to defeat phase when player fails.
     */
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void StartDefeatPhase();

    /**
     * Get the current game phase.
     * @return Current phase enumeration
     */
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    EGamePhase GetCurrentPhase() const { return CurrentGamePhase; }

    /**
     * Reload the current map for restart functionality.
     */
    UFUNCTION()
    void ReloadCurrentMap() const;

    // Ready-up system

    /**
     * Get the current ready-up progress as a normalized value.
     * @return Progress from 0.0 to 1.0
     */
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    float GetReadyUpProgress() const { return ReadyUpProgress / RequiredReadyUpTime; }

    // IDebugInformationProvider Interface Implementation
    virtual FString GetDebugCategory() const override;
    virtual FString GetDebugInformation() const override;

protected:
    virtual void BeginPlay() override;

public:
    virtual void StartPlay() override;

protected:
    // Phase management internals

    /**
     * Internal function to handle phase transitions with proper cleanup.
     * @param NewPhase - Phase to transition to
     */
    void SetGamePhase(EGamePhase NewPhase);

    // Ready-up system internals

    /**
     * Start the ready-up progress when player holds input.
     */
    void StartReadyUp();

    /**
     * Check if currently in a phase that allows ready-up progression.
     * @return true if in preparation phase
     */
    bool IsInAPreparationPhase() const;

    /**
     * Cancel ready-up progress when player releases input.
     */
    void CancelReadyUp();

    /**
     * Update ready-up progress each frame during input hold.
     * @param DeltaTime - Time elapsed since last update
     */
    void UpdateReadyUpProgress(float DeltaTime);

    // Entity management

    /**
     * Check if all enemy entities have been defeated.
     * @return true if no enemies remain
     */
    bool AreAllEnemiesDead() const;

    /**
     * Check if all crystal entities have been destroyed.
     * @return true if no crystals remain
     */
    bool AreAllCrystalsDead() const;

    /**
     * Handle entity removal events for phase transition logic.
     * @param Entity - Entity that was removed
     */
    UFUNCTION()
    void OnEntityRemoved(const TScriptInterface<IEntity>& Entity);

    /**
     * Bind to entity manager events for lifecycle tracking.
     */
    void BindToEntityEvents();

    // Manager references

    UPROPERTY(Transient)
    UManagerHandlerSubsystem* ManagerHandlerSubsystem;

    UPROPERTY(Transient)
    UWaveManager* WaveManager;

    UPROPERTY(Transient)
    UEntityManager* EntityManager;

    UPROPERTY(Transient)
    UCurrencyManager* CurrencyManager;


    // Configuration

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Currency")
    TSoftObjectPtr<UCurrencyManagerSettings> CurrencyManagerSettings;

    UPROPERTY(EditAnywhere, Category = "Game Flow")
    float RequiredReadyUpTime = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Game Flow")
    float DefeatPhaseDuration = 3.0f;

    UPROPERTY(EditAnywhere, Category = "Game Flow")
    float RewardPhaseDuration = 3.0f;

    // Runtime state

    UPROPERTY(BlueprintReadOnly, Category = "Game Flow")
    EGamePhase CurrentGamePhase;

    UPROPERTY()
    float ReadyUpProgress;

    UPROPERTY()
    bool bIsReadyingUp;

    FTimerHandle PhaseTransitionTimerHandle;
};
