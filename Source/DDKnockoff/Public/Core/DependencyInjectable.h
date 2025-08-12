#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DependencyInjectable.generated.h"

UINTERFACE(BlueprintType)
class DDKNOCKOFF_API UDependencyInjectable : public UInterface {
    GENERATED_BODY()
};

/**
 * Interface for actors that support dependency injection
 * 
 * Provides a consistent pattern for all actors that need dependencies:
 * 1. Check if dependencies are manually injected (for testing)
 * 2. If not, automatically get them from default sources (for normal gameplay)
 * 3. Validate that all required dependencies are available
 * 
 * Usage in BeginPlay():
 * ```cpp
 * void AMyActor::BeginPlay()
 * {
 *     Super::BeginPlay();
 *     
 *     // Handle dependency injection with single call
 *     EnsureDependenciesInjected();
 *     
 *     // Continue with normal BeginPlay logic...
 * }
 * ```
 * 
 * For testing, use SpawnActorDeferred:
 * ```cpp
 * AMyActor* Actor = World->SpawnActorDeferred<AMyActor>(ActorClass, Transform);
 * Actor->SetDependencies(MockDependencies);
 * UGameplayStatics::FinishSpawningActor(Actor, Transform);
 * ```
 */
class DDKNOCKOFF_API IDependencyInjectable {
    GENERATED_BODY()

public:
    /**
     * Check if all required dependencies have been manually injected
     * Used to determine whether to fall back to default source lookup
     * @return true if all dependencies are available, false if default source fallback is needed
     */
    virtual bool HasRequiredDependencies() const = 0;

    /**
     * Inject dependencies from their default sources (subsystems, components, etc.)
     * This is the fallback behavior for normal gameplay when dependencies
     * haven't been manually injected (e.g., for testing)
     */
    virtual void CollectDependencies() = 0;

    /**
     * Convenience method that handles the complete dependency injection flow.
     * Call this from BeginPlay() instead of manually implementing the pattern.
     * 
     * Performs:
     * 1. Check if dependencies are manually injected
     * 2. If not, inject from default sources
     * 3. Validate all dependencies are available
     */
    void EnsureDependenciesInjected() {
        if (!HasRequiredDependencies()) { CollectDependencies(); }

        // Validate dependencies
        ensureAlways(HasRequiredDependencies());
    }
};
