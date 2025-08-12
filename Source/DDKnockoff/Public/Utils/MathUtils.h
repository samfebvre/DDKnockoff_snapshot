#pragma once

/**
 * Advanced mathematical utility functions for smooth interpolation and animation.
 * Provides Unity-style SmoothDamp functions for natural movement and rotation transitions.
 */
class MathUtils {
public:
    /**
     * Smoothly interpolate between current and target values with velocity tracking.
     * Uses a spring-damper system to create natural easing without overshoot.
     * @param Current - Current value
     * @param Target - Target value to reach
     * @param CurrentVelocity - Reference to velocity state (modified by function)
     * @param SmoothTime - Approximate time to reach target
     * @param DeltaTime - Time elapsed since last call
     * @param MaxSpeed - Maximum speed limit for the interpolation
     * @return Smoothly interpolated value
     */
    static double SmoothDamp(
        double Current,
        double Target,
        double& CurrentVelocity,
        float SmoothTime,
        float DeltaTime,
        float MaxSpeed = FLT_MAX) {
        SmoothTime = FMath::Max(0.0001f, SmoothTime);
        const float Omega = 2.0f / SmoothTime;

        const float X = Omega * DeltaTime;
        const float Exp = 1.0f / (1.0f + X + 0.48f * X * X + 0.235f * X * X * X);

        float Change = Current - Target;
        const float OriginalTo = Target;

        // Clamp maximum speed
        const float MaxChange = MaxSpeed * SmoothTime;
        Change = FMath::Clamp(Change, -MaxChange, MaxChange);
        Target = Current - Change;

        const float Temp = (CurrentVelocity + Omega * Change) * DeltaTime;
        CurrentVelocity = (CurrentVelocity - Omega * Temp) * Exp;

        float Output = Target + (Change + Temp) * Exp;

        // Prevent overshooting
        if (OriginalTo - Current > 0.0f == Output > OriginalTo) {
            Output = OriginalTo;
            CurrentVelocity = (Output - OriginalTo) / DeltaTime;
        }

        return Output;
    }

    /**
     * Smoothly interpolate between rotator values with proper angle wrapping.
     * Handles angular differences correctly and prevents rotation through the long path.
     * @param Current - Current rotation
     * @param Target - Target rotation to reach
     * @param CurrentVelocity - Reference to rotational velocity state
     * @param SmoothTime - Approximate time to reach target
     * @param DeltaTime - Time elapsed since last call
     * @param MaxSpeed - Maximum rotational speed limit
     * @return Smoothly interpolated rotation
     */
    static FRotator SmoothDampRotator(
        const FRotator& Current,
        const FRotator& Target,
        FRotator& CurrentVelocity,
        float SmoothTime,
        float DeltaTime,
        float MaxSpeed = FLT_MAX) {
        auto SmoothDampAngle = [](double Current,
                                  double Target,
                                  double& Velocity,
                                  float SmoothTime,
                                  float DeltaTime,
                                  float MaxSpeed) {
            // Normalize the angle difference to [-180, 180]
            const float Delta = FMath::FindDeltaAngleDegrees(Current, Target);
            const float Result = SmoothDamp(Current,
                                            Current + Delta,
                                            Velocity,
                                            SmoothTime,
                                            DeltaTime,
                                            MaxSpeed);
            return Result;
        };

        FRotator Result;
        Result.Pitch = SmoothDampAngle(Current.Pitch,
                                       Target.Pitch,
                                       CurrentVelocity.Pitch,
                                       SmoothTime,
                                       DeltaTime,
                                       MaxSpeed);
        Result.Yaw = SmoothDampAngle(Current.Yaw,
                                     Target.Yaw,
                                     CurrentVelocity.Yaw,
                                     SmoothTime,
                                     DeltaTime,
                                     MaxSpeed);
        Result.Roll = SmoothDampAngle(Current.Roll,
                                      Target.Roll,
                                      CurrentVelocity.Roll,
                                      SmoothTime,
                                      DeltaTime,
                                      MaxSpeed);

        return Result;
    }
};
