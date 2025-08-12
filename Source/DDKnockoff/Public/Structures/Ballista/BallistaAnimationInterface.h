#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BallistaEnums.h"
#include "BallistaAnimationInterface.generated.h"

UINTERFACE(BlueprintType)
class DDKNOCKOFF_API UBallistaAnimationInterface : public UInterface {
    GENERATED_BODY()
};

/**
 * Interface for ballista animation functionality
 * Allows both real UBallistaAnimInstance and test mocks to be used interchangeably
 */
class DDKNOCKOFF_API IBallistaAnimationInterface {
    GENERATED_BODY()

public:
    virtual void PointToTargetLocation(const FVector& TargetLocation) = 0;
    virtual void PlayAttackAnim() = 0;
    virtual EBallistaAnimationState GetCurrentBallistaAnimationState() const = 0;
};
