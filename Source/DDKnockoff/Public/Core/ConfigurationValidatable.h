#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ConfigurationValidatable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UConfigurationValidatable : public UInterface {
    GENERATED_BODY()
};

/**
 * Interface for objects that need to validate their editor-configured properties.
 * This is separate from dependency injection validation and focuses on UPROPERTY
 * values that are set in the editor or Blueprint.
 */
class DDKNOCKOFF_API IConfigurationValidatable {
    GENERATED_BODY()

public:
    /**
     * Validates that all required editor-configured properties have valid values.
     * Should be called during BeginPlay after all properties have been set.
     */
    virtual void ValidateConfiguration() const = 0;
};
