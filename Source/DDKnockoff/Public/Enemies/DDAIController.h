#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyCharacterEnums.h"
#include "Core/ConfigurationValidatable.h"
#include "DDAIController.generated.h"

class ADDAICharacter;
class IEntity;

/**
 * AI controller for enemy characters with state-based behavior and target acquisition.
 * Manages pathfinding, combat decisions, and target prioritization for defensive structure destruction.
 * Features knockback response and dynamic state transitions based on environment conditions.
 */
UCLASS()
class DDKNOCKOFF_API ADDAIController : public AAIController, public IConfigurationValidatable {
    GENERATED_BODY()

public:
    ADDAIController(const FObjectInitializer& ObjectInitializer);

    // IConfigurationValidatable Interface Implementation
    virtual void ValidateConfiguration() const override;

protected:
    // Actor lifecycle
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Configuration

    UPROPERTY(EditDefaultsOnly, Category = "AI|Movement")
    float AcceptanceRadius = 5.0f;

    UPROPERTY(EditDefaultsOnly, Category = "AI|Targeting")
    float TargetUpdateInterval = 2.0f;

private:
    // Event handlers

    /**
     * Handle character knockback events to update AI state.
     */
    UFUNCTION()
    void OnCharacterTookKnockback();

    // State management

    /**
     * Process current AI state and execute appropriate behavior.
     */
    void ProcessCurrentState();

    /**
     * Handle behavior during idle state.
     */
    void HandleIdleState();

    /**
     * Handle behavior during movement state.
     */
    void HandleMovingState();

    /**
     * Handle behavior during attacking state.
     */
    void HandleAttackingState();

    /**
     * Transition to a new AI state with proper cleanup.
     * @param NewState - State to transition to
     */
    void TransitionToState(EEnemyAIState NewState);

    // State validation

    /**
     * Check if AI can make decisions (not stunned, valid character, etc.).
     * @return true if decisions can be made
     */
    bool CanMakeDecisions() const;

    /**
     * Check if targeting should be updated based on time interval.
     * @return true if targeting update is needed
     */
    bool ShouldUpdateTargeting() const;

    /**
     * Check if character is overlapping with defensive structures.
     * @return true if overlapping with defense
     */
    bool CharacterIsOverlappingADefense() const;

    // Combat

    /**
     * Attempt to attack the target if conditions are met.
     * @param Target - Actor to attack
     */
    void AttackIfAble(AActor& Target) const;

    // Movement

    /**
     * Stop all current movement and pathfinding operations.
     */
    void StopPathingAndMovement();

    /**
     * Initiate movement toward current target.
     */
    void MoveToTarget();

    /**
     * Check if pathfinding component is in idle state.
     * @return true if path component is idle
     */
    bool CheckIfPathComponentIsIdle() const;

    // Target acquisition

    /**
     * Update current target based on priority and availability.
     */
    void UpdateTarget();

    /**
     * Validate current target is still valid and accessible.
     * @return true if target is valid
     */
    bool TargetIsValid() const;

    // Runtime state

    UPROPERTY(Transient)
    ADDAICharacter* AICharacter;

    UPROPERTY(Transient)
    TWeakObjectPtr<AActor> TargetStructure;

    EEnemyAIState CurrentAIState;
    float LastTargetUpdateTime = 0.0f;
};
