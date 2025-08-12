#pragma once

/**
 * Overall validity state for structure placement validation.
 */
UENUM(BlueprintType)
enum class EStructurePlacementValidityState : uint8 {
    None UMETA(DisplayName = "None"),
    // Uninitialized state
    Invalid UMETA(DisplayName = "Invalid"),
    // Placement is not allowed
    Valid UMETA(DisplayName = "Valid"),
    // Placement is allowed
    MAX UMETA(Hidden)
};

/**
 * Specific reasons why structure placement might be invalid.
 * Uses bitfield flags to support multiple simultaneous reasons.
 */
UENUM(BlueprintType)
enum class EStructurePlacementInvalidityReason : uint8 {
    None = 0 UMETA(DisplayName = "None"),
    // No issues detected
    NoGroundFound = 1 << 0 UMETA(DisplayName = "No Ground Found"),
    // No valid ground surface
    Distance = 1 << 1 UMETA(DisplayName = "Distance"),
    // Too far from player
    Angle = 1 << 2 UMETA(DisplayName = "Angle"),
    // Invalid surface angle
    Overlapping = 1 << 3 UMETA(DisplayName = "Overlapping"),
    // Colliding with existing objects
    CannotAfford = 1 << 4 UMETA(DisplayName = "Cannot Afford"),
    // Insufficient currency
};

ENUM_CLASS_FLAGS(EStructurePlacementInvalidityReason)

/**
 * Preview interaction state for structure placement UI.
 */
UENUM(BlueprintType)
enum class EPreviewStructurePlacementState : uint8 {
    None UMETA(DisplayName = "None"),
    // No preview active
    Previewing_Position UMETA(DisplayName = "Previewing Position"),
    // Positioning phase
    Previewing_Rotation UMETA(DisplayName = "Previewing Rotation"),
    // Rotation adjustment phase
    MAX UMETA(Hidden)
};

/**
 * Basic placement state for structure lifecycle tracking.
 */
UENUM(BlueprintType)
enum class EStructurePlacementState : uint8 {
    Previewing UMETA(DisplayName = "Previewing"),
    // Structure is in preview mode
    NotPreviewing UMETA(DisplayName = "Placed"),
    // Structure has been placed
    MAX UMETA(Hidden)
};
