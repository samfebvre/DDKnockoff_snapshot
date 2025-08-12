#pragma once

#include "CoreMinimal.h"
#include "Tests/Common/TestUtils.h"
#include "Core/ManagerHandlerSubsystem.h"

/**
 * Base class for spec tests
 * Contains common boilerplate that should not be copied to functional tests
 */
class DDKNOCKOFFTESTS_API BaseSpec {
public:
    // SPEC_BOILERPLATE_BEGIN
    TObjectPtr<UTestWorldEngineSubsystem> Subsystem;
    TUniquePtr<FTestWorldHelper> WorldHelper;

    void SetupBaseSpecEnvironment(const TArray<UClass*>& Managers) {
        // Get the subsystem if not already cached
        if (!Subsystem) { Subsystem = GEngine->GetEngineSubsystem<UTestWorldEngineSubsystem>(); }

        // Create world using the standard pattern
        WorldHelper = MakeUnique<FTestWorldHelper>(Subsystem->GetPrivateWorldHelper());

        WorldHelper->GetWorld()->GetSubsystem<UManagerHandlerSubsystem>()->SetManagers(Managers);
    }

    void TeardownBaseSpecEnvironment() {
        WorldHelper->GetWorld()->GetSubsystem<UManagerHandlerSubsystem>()->ClearManagers();
        // Reset the world helper, which will trigger proper cleanup
        WorldHelper.Reset();
    }

    // SPEC_BOILERPLATE_END
};
