#pragma once

/**
 * Faction enumeration for entity relationships and targeting systems.
 * Used throughout the game to determine combat interactions and AI behavior.
 */
UENUM(BlueprintType)
enum class EFaction : uint8 {
    None UMETA(DisplayName = "None"),
    // No faction assigned
    Player UMETA(DisplayName = "Player"),
    // Player and allied structures
    Enemy UMETA(DisplayName = "Enemy"),
    // Enemy AI characters

    MAX UMETA(Hidden)
};
