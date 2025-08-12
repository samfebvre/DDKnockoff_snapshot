#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GeometryUtils.generated.h"

/**
 * Data structure containing calculated targeting information for turret systems.
 * Eliminates duplicated turret targeting logic across defensive structures.
 */
USTRUCT(BlueprintType)
struct DDKNOCKOFF_API FTurretTargetingData {
    GENERATED_BODY()

    /** Horizontal rotation for the turret body */
    UPROPERTY(BlueprintReadOnly)
    FRotator BodyRotation = FRotator::ZeroRotator;

    /** Vertical elevation angle for barrel */
    UPROPERTY(BlueprintReadOnly)
    float ElevationAngle = 0.0f;

    /** Whether targeting calculation was successful */
    UPROPERTY(BlueprintReadOnly)
    bool bValidTarget = false;
};

/**
 * Static utility library for geometric calculations and spatial relationships.
 * Provides common mathematical operations used throughout the game systems.
 */
UCLASS()
class DDKNOCKOFF_API UGeometryUtils : public UBlueprintFunctionLibrary {
    GENERATED_BODY()

public:
    // Vector operations

    /**
     * Remove Z component from a vector, optionally setting a specific Z value.
     * @param Vector - Vector to flatten
     * @param ZValue - Z component to set (default: 0.0f)
     * @return Flattened vector
     */
    UFUNCTION(BlueprintCallable, Category = "Geometry|Vector")
    static FVector FlattenVector(const FVector& Vector, float ZValue = 0.0f);

    /**
     * Calculate the angle between two vectors.
     * @param Vector1 - First vector
     * @param Vector2 - Second vector
     * @param bInDegrees - Return angle in degrees (true) or radians (false)
     * @return Angle between vectors
     */
    UFUNCTION(BlueprintCallable, Category = "Geometry|Vector")
    static float CalculateAngleBetweenVectors(const FVector& Vector1,
                                              const FVector& Vector2,
                                              bool bInDegrees = true);

    /**
     * Calculate elevation angle from one point to another.
     * @param From - Starting position
     * @param To - Target position
     * @param bInDegrees - Return angle in degrees (true) or radians (false)
     * @return Elevation angle
     */
    UFUNCTION(BlueprintCallable, Category = "Geometry|Vector")
    static float CalculateElevationAngle(const FVector& From,
                                         const FVector& To,
                                         bool bInDegrees = true);

    // Actor-based calculations

    /**
     * Calculate normalized direction vector from one actor to another.
     * @param From - Source actor
     * @param To - Target actor
     * @param bFlatten - Whether to ignore vertical component
     * @return Normalized direction vector
     */
    UFUNCTION(BlueprintCallable, Category = "Geometry|Actor")
    static FVector CalculateDirectionBetweenActors(AActor* From, AActor* To, bool bFlatten = false);

    /**
     * Get 3D distance between two actors.
     * @param Actor1 - First actor
     * @param Actor2 - Second actor
     * @param bSquared - Return squared distance for performance (avoids sqrt)
     * @return Distance between actors
     */
    UFUNCTION(BlueprintCallable, Category = "Geometry|Actor")
    static float GetDistanceBetweenActors(AActor* Actor1, AActor* Actor2, bool bSquared = false);

    /**
     * Get squared distance between two actors (more performant than regular distance).
     * @param Actor1 - First actor
     * @param Actor2 - Second actor
     * @return Squared distance between actors
     */
    UFUNCTION(BlueprintCallable,
        Category = "Geometry|Actor",
        meta = (Keywords = "distance squared performance"))
    static float GetDistanceSquaredBetweenActors(AActor* Actor1, AActor* Actor2);

    /**
     * Get horizontal (2D) distance between actors, ignoring vertical separation.
     * @param Actor1 - First actor
     * @param Actor2 - Second actor
     * @param bSquared - Return squared distance for performance
     * @return Flat distance between actors
     */
    UFUNCTION(BlueprintCallable,
        Category = "Geometry|Actor",
        meta = (Keywords = "distance flat 2d horizontal"))
    static float
    GetFlatDistanceBetweenActors(AActor* Actor1, AActor* Actor2, bool bSquared = false);

    // Range and vision checking

    /**
     * Check if an actor is within a specified range of another actor.
     * @param Actor - Source actor
     * @param Target - Target actor to check
     * @param Range - Maximum range
     * @param bUseSquared - Use squared distance for performance
     * @return true if target is within range
     */
    UFUNCTION(BlueprintCallable,
        Category = "Geometry|Range",
        meta = (Keywords = "distance range check within"))
    static bool IsActorWithinRange(AActor* Actor,
                                   AActor* Target,
                                   float Range,
                                   bool bUseSquared = false);

    /**
     * Check if target actor is within observer's vision cone.
     * @param Observer - Actor performing the observation
     * @param Target - Actor being observed
     * @param ConeAngleDegrees - Full angle of the vision cone in degrees
     * @param bIgnoreVertical - Whether to ignore vertical angle (horizontal-only check)
     * @return true if target is within vision cone
     */
    UFUNCTION(BlueprintCallable, Category = "Geometry|Vision")
    static bool IsActorInVisionCone(AActor* Observer,
                                    AActor* Target,
                                    float ConeAngleDegrees,
                                    bool bIgnoreVertical = true);

    /**
     * Check if two actors have unobstructed line of sight.
     * @param WorldContextObject - World context for trace
     * @param Observer - Actor checking line of sight
     * @param Target - Actor being observed
     * @param bTraceComplex - Use complex collision for trace
     * @return true if line of sight exists
     */
    UFUNCTION(BlueprintCallable,
        Category = "Geometry|Vision",
        meta = (Keywords = "line sight visibility trace", CallableWithoutWorldContext, WorldContext
            = "WorldContextObject"))
    static bool HasLineOfSight(const UObject* WorldContextObject,
                               AActor* Observer,
                               AActor* Target,
                               bool bTraceComplex = false);

    // Specialized calculations

    /**
     * Calculate complete turret targeting data for aiming at a target.
     * Provides both horizontal rotation and vertical elevation angles.
     * @param TurretLocation - Position of the turret
     * @param TargetLocation - Position of the target
     * @return Targeting data structure with rotation and elevation
     */
    UFUNCTION(BlueprintCallable, Category = "Geometry|Turret")
    static FTurretTargetingData CalculateTurretTargeting(
        const FVector& TurretLocation,
        const FVector& TargetLocation
        );
};
