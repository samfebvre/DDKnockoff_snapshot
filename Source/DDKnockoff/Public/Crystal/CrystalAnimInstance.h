#pragma once

#include "CoreMinimal.h"
#include "Enemies/AIAnimInstance.h"
#include "Core/ConfigurationValidatable.h"
#include "CrystalAnimInstance.generated.h"

/**
 * Animation instance for crystal entities with floating and rotation animations.
 * Provides smooth bobbing motion and continuous rotation for crystal objects.
 * Configurable animation speeds and amplitudes for visual appeal.
 */
UCLASS()
class DDKNOCKOFF_API
    UCrystalAnimInstance : public UAIAnimInstance, public IConfigurationValidatable {
    GENERATED_BODY()

public:
    // UAnimInstance Interface
    virtual void NativeBeginPlay() override;

    // Animation system

    /**
     * Update crystal animation properties based on time.
     * @param DeltaTime - Time elapsed since last update
     */
    void UpdateAnimationProperties(float DeltaTime);

    // IConfigurationValidatable Interface
    virtual void ValidateConfiguration() const override;

protected:
    // Animation configuration

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BobSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BobHeight = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float RotationSpeed = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
    float InitialVerticalOffset = 0.0f;

    // Runtime state

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    float CurrentVerticalOffset = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
    float CurrentYaw = 0.0f;
};
