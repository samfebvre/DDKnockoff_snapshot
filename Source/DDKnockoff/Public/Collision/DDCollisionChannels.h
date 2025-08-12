#pragma once

/**
 * Custom collision channels used throughout the DD Knockoff game.
 * Maps custom collision requirements to Unreal's GameTraceChannel slots.
 */
namespace DDCollisionChannels {
// Combat collision channels
constexpr ECollisionChannel ECC_Hurtbox = ECC_GameTraceChannel1; // Enemy character damage reception
constexpr ECollisionChannel ECC_Hitbox = ECC_GameTraceChannel2;  // Enemy character damage dealing

// Structure collision channels  
constexpr ECollisionChannel ECC_PreviewStructure = ECC_GameTraceChannel3;
// Structure placement previews

// Interaction collision channels
constexpr ECollisionChannel ECC_Trigger = ECC_GameTraceChannel4;  // General trigger areas
constexpr ECollisionChannel ECC_Interact = ECC_GameTraceChannel5; // Player interaction objects
constexpr ECollisionChannel ECC_Collectibles = ECC_GameTraceChannel6;
// Currency and collectible items
constexpr ECollisionChannel ECC_EnemyPawn = ECC_GameTraceChannel7; // Enemy pawns
}
