#include "Animation/RootMotionExtractorModifier.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimCurveTypes.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Engine/Engine.h"

URootMotionExtractorModifier::URootMotionExtractorModifier() {
    SampleRate = 30.0f;
    bExtractTranslation = true;
    bExtractRotation = true;
    bExtractSpeed = true;
    bExtractVelocity = true;
    CurvePrefix = TEXT("RootMotion");
}

void URootMotionExtractorModifier::OnApply_Implementation(UAnimSequence* Animation) {
    if (!Animation || !Animation->GetSkeleton()) {
        UE_LOG(LogTemp,
               Warning,
               TEXT("RootMotionExtractorModifier: Invalid animation or skeleton"));
        return;
    }

    const float AnimLength = Animation->GetPlayLength();
    if (AnimLength <= 0.0f) {
        UE_LOG(LogTemp, Warning, TEXT("RootMotionExtractorModifier: Animation has no length"));
        return;
    }

    const int32 NumSamples = FMath::CeilToInt(AnimLength * SampleRate);

    // Arrays to store curve data
    TArray<FVector2D> TranslationXKeys;
    TArray<FVector2D> TranslationYKeys;
    TArray<FVector2D> TranslationZKeys;
    TArray<FVector2D> RotationYawKeys;
    TArray<FVector2D> SpeedKeys;
    TArray<FVector2D> VelocityXKeys;
    TArray<FVector2D> VelocityYKeys;
    TArray<FVector2D> VelocityZKeys;

    FTransform PreviousTransform = FTransform::Identity;
    float PreviousTime = 0.0f;
    bool bFirstFrame = true;

    UE_LOG(LogTemp,
           Log,
           TEXT("RootMotionExtractorModifier: Processing %s with %d samples"),
           *Animation->GetName(),
           NumSamples);

    for (int32 i = 0; i <= NumSamples; i++) {
        float Time = FMath::Min(i / SampleRate, AnimLength);

        // Extract root motion from start to current time
        FTransform CurrentRootMotion = Animation->ExtractRootMotion(0.0f, Time, false);

        if (!bFirstFrame) {
            // Calculate delta movement
            FTransform DeltaTransform = CurrentRootMotion.GetRelativeTransform(PreviousTransform);
            FVector DeltaTranslation = DeltaTransform.GetLocation();
            float DeltaTime = Time - PreviousTime;

            // Add translation keys (delta per frame)
            if (bExtractTranslation) {
                TranslationXKeys.Add(FVector2D(Time, DeltaTranslation.X));
                TranslationYKeys.Add(FVector2D(Time, DeltaTranslation.Y));
                TranslationZKeys.Add(FVector2D(Time, DeltaTranslation.Z));
            }

            // Add rotation keys (delta yaw per frame)
            if (bExtractRotation) {
                float DeltaYaw = DeltaTransform.GetRotation().Rotator().Yaw;
                RotationYawKeys.Add(FVector2D(Time, DeltaYaw));
            }

            // Add speed keys (units per second)
            if (bExtractSpeed) {
                float Speed = DeltaTime > 0.0f ? DeltaTranslation.Size() / DeltaTime : 0.0f;
                SpeedKeys.Add(FVector2D(Time, Speed));
            }

            // Add velocity keys (units per second in each axis)
            if (bExtractVelocity) {
                if (DeltaTime > 0.0f) {
                    FVector Velocity = DeltaTranslation / DeltaTime;
                    VelocityXKeys.Add(FVector2D(Time, Velocity.X));
                    VelocityYKeys.Add(FVector2D(Time, Velocity.Y));
                    VelocityZKeys.Add(FVector2D(Time, Velocity.Z));
                } else {
                    VelocityXKeys.Add(FVector2D(Time, 0.0f));
                    VelocityYKeys.Add(FVector2D(Time, 0.0f));
                    VelocityZKeys.Add(FVector2D(Time, 0.0f));
                }
            }
        } else {
            // First frame - add zero values
            if (bExtractTranslation) {
                TranslationXKeys.Add(FVector2D(Time, 0.0f));
                TranslationYKeys.Add(FVector2D(Time, 0.0f));
                TranslationZKeys.Add(FVector2D(Time, 0.0f));
            }
            if (bExtractRotation) { RotationYawKeys.Add(FVector2D(Time, 0.0f)); }
            if (bExtractSpeed) { SpeedKeys.Add(FVector2D(Time, 0.0f)); }
            if (bExtractVelocity) {
                VelocityXKeys.Add(FVector2D(Time, 0.0f));
                VelocityYKeys.Add(FVector2D(Time, 0.0f));
                VelocityZKeys.Add(FVector2D(Time, 0.0f));
            }
            bFirstFrame = false;
        }

        PreviousTransform = CurrentRootMotion;
        PreviousTime = Time;
    }

    // Add curves to the animation
    if (bExtractTranslation) {
        AddCurveToAnimation(Animation, GetCurveName(TEXT("DeltaX")), TranslationXKeys);
        AddCurveToAnimation(Animation, GetCurveName(TEXT("DeltaY")), TranslationYKeys);
        AddCurveToAnimation(Animation, GetCurveName(TEXT("DeltaZ")), TranslationZKeys);
    }

    if (bExtractRotation) {
        AddCurveToAnimation(Animation, GetCurveName(TEXT("DeltaYaw")), RotationYawKeys);
    }

    if (bExtractSpeed) { AddCurveToAnimation(Animation, GetCurveName(TEXT("Speed")), SpeedKeys); }

    if (bExtractVelocity) {
        AddCurveToAnimation(Animation, GetCurveName(TEXT("VelocityX")), VelocityXKeys);
        AddCurveToAnimation(Animation, GetCurveName(TEXT("VelocityY")), VelocityYKeys);
        AddCurveToAnimation(Animation, GetCurveName(TEXT("VelocityZ")), VelocityZKeys);
    }

    UE_LOG(LogTemp,
           Log,
           TEXT("RootMotionExtractorModifier: Completed extraction for %s"),
           *Animation->GetName());
}

void URootMotionExtractorModifier::OnRevert_Implementation(UAnimSequence* Animation) {
    if (!Animation) { return; }

    // Remove all curves we might have added
    TArray<FString> CurveNames = GetAllCurveNames();

    for (const FString& CurveName : CurveNames) { RemoveCurveFromAnimation(Animation, CurveName); }

    UE_LOG(LogTemp,
           Log,
           TEXT("RootMotionExtractorModifier: Removed root motion curves from %s"),
           *Animation->GetName());
}

void URootMotionExtractorModifier::AddCurveToAnimation(UAnimSequence* Animation,
                                                       const FString& CurveName,
                                                       const TArray<FVector2D>& KeyData) {
    if (!Animation || KeyData.Num() == 0) { return; }

    const FName CurveNameFName(*CurveName);

    // Get the animation data controller for UE5
    IAnimationDataController& Controller = Animation->GetController();

    // Add the curve
    const FAnimationCurveIdentifier CurveId(CurveNameFName, ERawCurveTrackTypes::RCT_Float);
    Controller.AddCurve(CurveId);

    // Convert key data to curve keys
    TArray<FRichCurveKey> CurveKeys;
    for (const FVector2D& Key : KeyData) {
        FRichCurveKey NewKey;
        NewKey.Time = Key.X;
        NewKey.Value = Key.Y;
        NewKey.InterpMode = RCIM_Linear;
        CurveKeys.Add(NewKey);
    }

    // Set the curve keys
    Controller.SetCurveKeys(CurveId, CurveKeys);

    UE_LOG(LogTemp,
           Verbose,
           TEXT("RootMotionExtractorModifier: Added curve %s with %d keys"),
           *CurveName,
           KeyData.Num());
}

void URootMotionExtractorModifier::RemoveCurveFromAnimation(UAnimSequence* Animation,
                                                            const FString& CurveName) {
    if (!Animation) { return; }

    const FName CurveNameFName(*CurveName);

    // Use the animation data controller for UE5
    IAnimationDataController& Controller = Animation->GetController();
    const FAnimationCurveIdentifier CurveId(CurveNameFName, ERawCurveTrackTypes::RCT_Float);
    Controller.RemoveCurve(CurveId);
}

FString URootMotionExtractorModifier::GetCurveName(const FString& Suffix) const {
    return FString::Printf(TEXT("%s_%s"), *CurvePrefix, *Suffix);
}

TArray<FString> URootMotionExtractorModifier::GetAllCurveNames() const {
    TArray<FString> CurveNames;

    if (bExtractTranslation) {
        CurveNames.Add(GetCurveName(TEXT("DeltaX")));
        CurveNames.Add(GetCurveName(TEXT("DeltaY")));
        CurveNames.Add(GetCurveName(TEXT("DeltaZ")));
    }

    if (bExtractRotation) { CurveNames.Add(GetCurveName(TEXT("DeltaYaw"))); }

    if (bExtractSpeed) { CurveNames.Add(GetCurveName(TEXT("Speed"))); }

    if (bExtractVelocity) {
        CurveNames.Add(GetCurveName(TEXT("VelocityX")));
        CurveNames.Add(GetCurveName(TEXT("VelocityY")));
        CurveNames.Add(GetCurveName(TEXT("VelocityZ")));
    }

    return CurveNames;
}
