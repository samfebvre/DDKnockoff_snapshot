#include "Tests/Common/TestUtils.h"

void FTestUtils::TickMultipleFrames(const FTestWorldHelper* WorldHelper,
                                    int32 FrameCount,
                                    float DeltaTime) {
    for (int32 i = 0; i < FrameCount; ++i) {
        // Use the WorldHelper to tick the entire world
        WorldHelper->Tick(DeltaTime);
        UE_LOG(LogTemp,
               VeryVerbose,
               TEXT("FTestUtils::TickMultipleFrames - Frame %d/%d"),
               i + 1,
               FrameCount);
    }
}

bool FTestUtils::WaitForCondition(const FTestWorldHelper* WorldHelper,
                                  const TFunction<bool()>& ConditionFunction,
                                  float TimeoutSeconds,
                                  const FString& DescriptionForLogging) {
    constexpr float DeltaTime = 0.016f; // ~60 FPS - consistent with TickMultipleFrames
    const int32 MaxFrames = FMath::CeilToInt(TimeoutSeconds / DeltaTime);

    UE_LOG(LogTemp,
           Log,
           TEXT(
               "FTestUtils::WaitForCondition - Starting wait for '%s' (timeout: %.2f seconds, max frames: %d)"
           ),
           *DescriptionForLogging,
           TimeoutSeconds,
           MaxFrames);

    for (int32 Frame = 0; Frame < MaxFrames; ++Frame) {
        // Tick the world to allow game systems to update
        WorldHelper->Tick(DeltaTime);

        // Check if the condition is met
        if (ConditionFunction()) {
            const float ElapsedTime = (Frame + 1) * DeltaTime;
            UE_LOG(LogTemp,
                   Log,
                   TEXT(
                       "FTestUtils::WaitForCondition - Condition '%s' met after %d frames (%.2f seconds)"
                   ),
                   *DescriptionForLogging,
                   Frame + 1,
                   ElapsedTime);
            return true;
        }
    }

    UE_LOG(LogTemp,
           Warning,
           TEXT(
               "FTestUtils::WaitForCondition - Timeout waiting for condition '%s' after %.2f seconds"
           ),
           *DescriptionForLogging,
           TimeoutSeconds);
    return false;
}
