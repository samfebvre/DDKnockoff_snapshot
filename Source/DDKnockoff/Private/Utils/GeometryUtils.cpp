#include "Utils/GeometryUtils.h"

FVector UGeometryUtils::FlattenVector(const FVector& Vector, float ZValue) {
    return FVector(Vector.X, Vector.Y, ZValue);
}

float UGeometryUtils::CalculateAngleBetweenVectors(const FVector& Vector1,
                                                   const FVector& Vector2,
                                                   bool bInDegrees) {
    const float DotProduct = FVector::DotProduct(Vector1.GetSafeNormal(), Vector2.GetSafeNormal());
    const float AngleRadians = FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f));

    return bInDegrees ? FMath::RadiansToDegrees(AngleRadians) : AngleRadians;
}

FVector UGeometryUtils::CalculateDirectionBetweenActors(AActor* From, AActor* To, bool bFlatten) {
    if (!From || !To) { return FVector::ZeroVector; }

    FVector FromLocation = From->GetActorLocation();
    FVector ToLocation = To->GetActorLocation();

    if (bFlatten) {
        FromLocation = FlattenVector(FromLocation);
        ToLocation = FlattenVector(ToLocation);
    }

    return (ToLocation - FromLocation).GetSafeNormal();
}

bool UGeometryUtils::IsActorInVisionCone(AActor* Observer,
                                         AActor* Target,
                                         float ConeAngleDegrees,
                                         bool bIgnoreVertical) {
    if (!Observer || !Target || ConeAngleDegrees >= 360.0f) { return true; }

    if (ConeAngleDegrees <= 0.0f) { return false; }

    // Get direction vectors
    FVector ObserverLocation = Observer->GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();
    FVector ObserverForward = Observer->GetActorForwardVector();

    if (bIgnoreVertical) {
        // Flatten for lateral-only check
        ObserverLocation = FlattenVector(ObserverLocation);
        TargetLocation = FlattenVector(TargetLocation);
        ObserverForward = FlattenVector(ObserverForward);
    }

    const FVector DirectionToTarget = (TargetLocation - ObserverLocation).GetSafeNormal();
    const float Angle = CalculateAngleBetweenVectors(ObserverForward, DirectionToTarget, true);

    // Check if within half-cone angle
    return Angle <= ConeAngleDegrees * 0.5f;
}

float UGeometryUtils::CalculateElevationAngle(const FVector& From,
                                              const FVector& To,
                                              bool bInDegrees) {
    const FVector DirectionVector = To - From;
    const FVector FlatDirection = FlattenVector(DirectionVector);

    const float AngleRadians = FMath::Acos(
        FVector::DotProduct(DirectionVector.GetSafeNormal(), FlatDirection.GetSafeNormal()));

    return bInDegrees ? FMath::RadiansToDegrees(AngleRadians) : AngleRadians;
}

float UGeometryUtils::GetDistanceBetweenActors(AActor* Actor1, AActor* Actor2, bool bSquared) {
    if (!Actor1 || !Actor2) {
        return -1.0f; // Invalid
    }

    const FVector Location1 = Actor1->GetActorLocation();
    const FVector Location2 = Actor2->GetActorLocation();

    return bSquared
               ? FVector::DistSquared(Location1, Location2)
               : FVector::Dist(Location1, Location2);
}

FTurretTargetingData UGeometryUtils::CalculateTurretTargeting(
    const FVector& TurretLocation,
    const FVector& TargetLocation) {
    FTurretTargetingData Result;

    // Validate inputs
    if (TurretLocation.Equals(TargetLocation)) {
        Result.bValidTarget = false;
        return Result;
    }

    // Calculate direction to target
    const FVector DirectionToTarget = TargetLocation - TurretLocation;

    // Calculate horizontal (body) rotation
    const FVector FlatDirection = FlattenVector(DirectionToTarget);
    if (!FlatDirection.IsNearlyZero()) {
        Result.BodyRotation = FlatDirection.Rotation();
        Result.bValidTarget = true;
    } else {
        Result.bValidTarget = false;
        return Result;
    }

    // Calculate elevation angle
    Result.ElevationAngle = CalculateElevationAngle(TurretLocation, TargetLocation, true);

    return Result;
}

bool UGeometryUtils::IsActorWithinRange(AActor* Actor,
                                        AActor* Target,
                                        float Range,
                                        bool bUseSquared) {
    if (!Actor || !Target || Range <= 0.0f) { return false; }

    const float Distance = GetDistanceBetweenActors(Actor, Target, bUseSquared);
    const float CompareRange = bUseSquared ? Range * Range : Range;

    return Distance >= 0.0f && Distance <= CompareRange;
}

float UGeometryUtils::GetDistanceSquaredBetweenActors(AActor* Actor1, AActor* Actor2) {
    return GetDistanceBetweenActors(Actor1, Actor2, true);
}

float UGeometryUtils::GetFlatDistanceBetweenActors(AActor* Actor1, AActor* Actor2, bool bSquared) {
    if (!Actor1 || !Actor2) {
        return -1.0f; // Invalid
    }

    const FVector Location1 = FlattenVector(Actor1->GetActorLocation());
    const FVector Location2 = FlattenVector(Actor2->GetActorLocation());

    return bSquared
               ? FVector::DistSquared(Location1, Location2)
               : FVector::Dist(Location1, Location2);
}

bool UGeometryUtils::HasLineOfSight(const UObject* WorldContextObject,
                                    AActor* Observer,
                                    AActor* Target,
                                    bool bTraceComplex) {
    if (!Observer || !Target || !WorldContextObject) { return false; }

    const UWorld* World = WorldContextObject->GetWorld();
    if (!World) { return false; }

    const FVector ObserverLocation = Observer->GetActorLocation();
    const FVector TargetLocation = Target->GetActorLocation();

    // Set up collision query params
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Observer);
    QueryParams.AddIgnoredActor(Target);
    QueryParams.bTraceComplex = bTraceComplex;

    // Perform line trace
    FHitResult HitResult;
    const bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        ObserverLocation,
        TargetLocation,
        ECC_Visibility,
        QueryParams
        );

    // If no hit, we have line of sight
    return !bHit;
}
