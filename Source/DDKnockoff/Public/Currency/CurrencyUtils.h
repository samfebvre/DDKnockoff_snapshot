#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "CurrencyUtils.generated.h"

class UCurrencyManager;
class UCurrencySpawner;

/**
 * Static utility library for currency spawning and management operations.
 * Provides convenient methods for spawning currency crystals with physics and distribution.
 */
UCLASS()
class DDKNOCKOFF_API UCurrencyUtils : public UBlueprintFunctionLibrary {
    GENERATED_BODY()

public:
    /**
     * Spawn a burst of currency crystals using the standard distribution pattern.
     * @param World - World context for spawning
     * @param CurrencyManager - Manager to calculate crystal distribution
     * @param CurrencySpawner - Spawner component to handle physics and positioning
     * @param CurrencyAmount - Total currency value to spawn
     * @param MinimumCrystalCount - Minimum number of crystals to spawn
     * @param SpawnDirection - Direction to launch crystals (normalized)
     * @param LaunchSpeed - Initial velocity for crystal physics
     * @return true if spawning succeeded
     */
    UFUNCTION(BlueprintCallable, Category = "Currency")
    static bool SpawnCurrencyBurst(
        UWorld* World,
        UCurrencyManager* CurrencyManager,
        UCurrencySpawner* CurrencySpawner,
        int32 CurrencyAmount,
        int32 MinimumCrystalCount,
        const FVector& SpawnDirection = FVector::UpVector,
        float LaunchSpeed = 300.0f
        );

    /**
     * Convenience method for spawning currency from an actor context.
     * Automatically retrieves the currency manager from the world.
     * @param SourceActor - Actor providing world context and spawn location
     * @param CurrencySpawner - Spawner component to handle physics and positioning
     * @param CurrencyAmount - Total currency value to spawn
     * @param MinimumCrystalCount - Minimum number of crystals to spawn
     * @param SpawnDirection - Direction to launch crystals (normalized)
     * @param LaunchSpeed - Initial velocity for crystal physics
     * @return true if spawning succeeded
     */
    UFUNCTION(BlueprintCallable, Category = "Currency")
    static bool SpawnCurrencyBurstFromActor(
        AActor* SourceActor,
        UCurrencySpawner* CurrencySpawner,
        int32 CurrencyAmount,
        int32 MinimumCrystalCount,
        const FVector& SpawnDirection = FVector::UpVector,
        float LaunchSpeed = 300.0f
        );
};
