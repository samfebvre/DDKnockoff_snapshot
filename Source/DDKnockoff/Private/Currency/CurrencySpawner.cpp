#include "Currency/CurrencySpawner.h"


// Sets default values for this component's properties
UCurrencySpawner::UCurrencySpawner() {
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;

    // ...
}


// Called when the game starts
void UCurrencySpawner::BeginPlay() {
    Super::BeginPlay();

    // Validate configuration
    ValidateConfiguration();
}


// Called every frame
void UCurrencySpawner::TickComponent(float DeltaTime,
                                     ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // ...
}

void UCurrencySpawner::SpawnCurrencyBurst(TSubclassOf<ACurrencyCrystal> CrystalClass,
                                          int32 Count,
                                          FVector Direction,
                                          float Speed) const {
    if (!GetWorld() || !CrystalClass || Count <= 0) { return; }

    // Get spawn location (component's world location)
    const FVector SpawnLocation = GetComponentLocation();

    // Normalize the direction vector
    Direction.Normalize();

    for (int32 i = 0; i < Count; i++) {
        // Calculate random spread angles
        const float HorizontalAngle = FMath::RandRange(-SpreadAngle, SpreadAngle);
        const float VerticalAngle = FMath::RandRange(-VerticalSpreadAngle, VerticalSpreadAngle);

        // Create a rotation from the direction vector
        FRotator BaseRotation = Direction.Rotation();

        // Apply random spread
        FRotator SpreadRotation = BaseRotation + FRotator(VerticalAngle, HorizontalAngle, 0.0f);
        FVector SpreadDirection = SpreadRotation.Vector();

        // Calculate random speed variation
        const float FinalSpeed = Speed * FMath::RandRange(1.0f - SpeedVariation,
                                                          1.0f + SpeedVariation);

        // Spawn the crystal
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        // calculate a random spawn rotation
        FRotator SpawnRotation = FRotator(FMath::RandRange(0.0f, 360.0f),
                                          FMath::RandRange(0.0f, 360.0f),
                                          FMath::RandRange(0.0f, 360.0f));

        // Spawn the crystal actor
        ACurrencyCrystal* Crystal = GetWorld()->SpawnActor<ACurrencyCrystal>(
            CrystalClass,
            SpawnLocation,
            SpawnRotation,
            SpawnParams);

        if (Crystal && Crystal->GetMesh()) {
            // Apply impulse to launch the crystal
            Crystal->GetMesh()->AddImpulse(SpreadDirection * FinalSpeed, NAME_None, true);

            // Apply rotational velocity for some spin
            FVector RandomRotationalVelocity = FVector(
                FMath::RandRange(-1.0f, 1.0f),
                FMath::RandRange(-1.0f, 1.0f),
                FMath::RandRange(-1.0f, 1.0f)
                );

            Crystal->GetMesh()->SetPhysicsAngularVelocityInDegrees(
                RandomRotationalVelocity * 1000.0f,
                false);
        }
    }
}

FVector UCurrencySpawner::GetUpForwardVector() const {
    // Return a vector that points up and forward relative to the component's orientation
    const auto Ret = GetComponentRotation().Vector() + FVector(0.0f, 0.0f, 1.2f);
    return Ret;
}

// IConfigurationValidatable interface implementation
void UCurrencySpawner::ValidateConfiguration() const {
    // Validate angle values are reasonable (0-180 degrees)
    ensureAlways(SpreadAngle >= 0.0f && SpreadAngle <= 180.0f);
    ensureAlways(VerticalSpreadAngle >= 0.0f && VerticalSpreadAngle <= 180.0f);

    // Validate speed variation is a reasonable percentage (0-1 = 0%-100%)
    ensureAlways(SpeedVariation >= 0.0f && SpeedVariation <= 1.0f);
}
