#pragma once

/**
 * AI behavior type enumeration for enemy character classification.
 */
UENUM(BlueprintType)
enum class EEnemyAIType : uint8 {
    None UMETA(DisplayName = "None"),
    // No AI behavior assigned
    MoveAround UMETA(DisplayName = "Move Around"),
    // Patrol or wander behavior
    Attack UMETA(DisplayName = "Attack"),
    // Aggressive combat behavior

    MAX UMETA(Hidden)
};
