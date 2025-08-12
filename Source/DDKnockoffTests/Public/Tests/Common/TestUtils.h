#pragma once

#include "CoreMinimal.h"
#include "TestWorldEngineSubsystem.h"

/**
 * Generic test utilities for DDKnockoff testing framework
 * Provides common functionality that can be used across all test types
 */
class DDKNOCKOFFTESTS_API FTestUtils {
public:
    // World simulation utilities
    static void TickMultipleFrames(const FTestWorldHelper* WorldHelper,
                                   int32 FrameCount = 5,
                                   float DeltaTime = 0.016f);

    // Generic conditional waiting - core utility for waiting until conditions are met
    static bool WaitForCondition(const FTestWorldHelper* WorldHelper,
                                 const TFunction<bool()>& ConditionFunction,
                                 float TimeoutSeconds = 10.0f,
                                 const FString& DescriptionForLogging = TEXT("condition"));

    // Entity damage waiting - specialized condition waiter for damage verification
    template <typename TEntity>
    static bool WaitForEntityDamage(const FTestWorldHelper* WorldHelper,
                                    const TEntity* Entity,
                                    float InitialHealth,
                                    float TimeoutSeconds = 10.0f,
                                    const FString& EntityDescription = TEXT("Entity"));
};

template <typename TEntity>
bool FTestUtils::WaitForEntityDamage(const FTestWorldHelper* WorldHelper,
                                     const TEntity* Entity,
                                     float InitialHealth,
                                     float TimeoutSeconds,
                                     const FString& EntityDescription) {
    const FString ConditionDescription = FString::Printf(TEXT("%s damage"), *EntityDescription);

    return WaitForCondition(WorldHelper,
                            [Entity, InitialHealth]() -> bool {
                                return Entity && Entity->GetCurrentHealth() < InitialHealth;
                            },
                            TimeoutSeconds,
                            ConditionDescription);
}
