#pragma once

/**
 * Animation state enumeration for enemy character pose control.
 */
UENUM(BlueprintType)
enum class EEnemyPoseState : uint8 {
    Locomotion UMETA(DisplayName = "Locomotion"),
    // Normal movement and idle animations
    Attack UMETA(DisplayName = "Attack"),
    // Combat attack animations
    HitReaction UMETA(DisplayName = "HitReaction"),
    // Damage response animations

    MAX UMETA(Hidden)
};

/**
 * AI state enumeration for enemy behavior control.
 */
UENUM(BlueprintType)
enum class EEnemyAIState : uint8 {
    None UMETA(DisplayName = "None"),
    // Idle or uninitialized state
    MovingTowardsTarget UMETA(DisplayName = "MovingTowardsTarget"),
    // Pathfinding to target structure
    AttackingTarget UMETA(DisplayName = "AttackingTarget"),
    // Engaging target in combat

    // TODO: Consider adding 'searching for target' state for more sophisticated AI
    MAX UMETA(Hidden)
};

/**
 * Overlap state enumeration for character collision detection.
 */
UENUM(BlueprintType)
enum class CharacterActorOverlapState : uint8 {
    None UMETA(DisplayName = "None"),
    // No special overlaps detected
    OverlappingStructure UMETA(DisplayName = "OverlappingDefense"),
    // Overlapping with defensive structure

    MAX UMETA(Hidden)
};
