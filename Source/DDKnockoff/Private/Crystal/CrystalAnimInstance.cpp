#include "Crystal/CrystalAnimInstance.h"

void UCrystalAnimInstance::UpdateAnimationProperties(float DeltaTime) {
    // Use the current game time for the sine wave to avoid accumulating float imprecisions
    const double GameTime = FPlatformTime::Seconds();

    // Calculate bobbing motion using a sine wave
    CurrentVerticalOffset = FMath::Sin(GameTime * BobSpeed) * BobHeight;
    CurrentVerticalOffset += InitialVerticalOffset;

    // Update rotation (continuous rotation around Z axis)
    CurrentYaw += RotationSpeed * DeltaTime;

    // Keep the yaw between 0 and 360 degrees
    if (CurrentYaw >= 360.0f) { CurrentYaw -= 360.0f; }
}

// UAnimInstance interface
void UCrystalAnimInstance::NativeBeginPlay() {
    Super::NativeBeginPlay();

    // Validate configuration during initialization
    ValidateConfiguration();
}

// IConfigurationValidatable interface implementation
void UCrystalAnimInstance::ValidateConfiguration() const {
    // Validate animation properties
    ensureAlways(BobSpeed > 0.0f);
    ensureAlways(BobHeight > 0.0f);
    ensureAlways(RotationSpeed > 0.0f);
}
