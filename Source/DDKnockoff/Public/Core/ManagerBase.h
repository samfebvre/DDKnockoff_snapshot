#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ManagerBase.generated.h"

/**
 * Base class for all manager systems in the game.
 * Provides standard lifecycle functions for initialization, ticking, and cleanup.
 * Used by subsystem managers for consistent behavior across different systems.
 */
UCLASS()
class DDKNOCKOFF_API UManagerBase : public UObject {
    GENERATED_BODY()

public:
    /**
     * Initialize the manager. Called during system startup.
     * Override in derived classes to set up manager-specific functionality.
     */
    virtual void Initialize();

    /**
     * Cleanup and deinitialize the manager. Called during system shutdown.
     * Override in derived classes to properly release resources.
     */
    virtual void Deinitialize();

    /**
     * Update manager state each frame.
     * @param DeltaTime - Time elapsed since last tick
     */
    virtual void Tick(float DeltaTime);
};
