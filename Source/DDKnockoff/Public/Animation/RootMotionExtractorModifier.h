#pragma once

#include "CoreMinimal.h"
#include "AnimationModifier.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimCurveTypes.h"
#include "RootMotionExtractorModifier.generated.h"

/**
 * Animation Modifier that extracts root motion data from animations and converts it to curves.
 * This allows you to use the root motion data in code while maintaining control over character movement.
 */
UCLASS(BlueprintType, Blueprintable)
class DDKNOCKOFF_API URootMotionExtractorModifier : public UAnimationModifier {
    GENERATED_BODY()

public:
    URootMotionExtractorModifier();
    
    UPROPERTY(EditAnywhere,
        BlueprintReadWrite,
        Category = "Settings",
        meta = (ClampMin = "1.0", ClampMax = "120.0"))
    float SampleRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bExtractTranslation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bExtractRotation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bExtractSpeed = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bExtractVelocity = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Curve Names")
    FString CurvePrefix = TEXT("RootMotion");

    // The main functions that process the animation
    virtual void OnApply_Implementation(UAnimSequence* Animation) override;
    virtual void OnRevert_Implementation(UAnimSequence* Animation) override;

private:
    static void AddCurveToAnimation(UAnimSequence* Animation,
                                    const FString& CurveName,
                                    const TArray<FVector2D>& KeyData);
    static void RemoveCurveFromAnimation(UAnimSequence* Animation, const FString& CurveName);

    // Helper functions
    FString GetCurveName(const FString& Suffix) const;
    TArray<FString> GetAllCurveNames() const;
};
