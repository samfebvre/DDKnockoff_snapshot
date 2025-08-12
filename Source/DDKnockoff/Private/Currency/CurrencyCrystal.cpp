#include "Currency/CurrencyCrystal.h"

#include "Collision/DDCollisionChannels.h"


// Sets default values
ACurrencyCrystal::ACurrencyCrystal() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Initialize rotation axis with a random direction
    CurrentRotationAxis = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f)
        ).GetSafeNormal();

    // Setup mesh
    CrystalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrystalMesh"));
    RootComponent = CrystalMesh;
    CrystalMesh->SetCollisionObjectType(DDCollisionChannels::ECC_Collectibles);
    CrystalMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    CrystalMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    CrystalMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    CrystalMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    CrystalMesh->SetCollisionResponseToChannel(DDCollisionChannels::ECC_Trigger, ECR_Overlap);
    CrystalMesh->SetGenerateOverlapEvents(true);
    CrystalMesh->SetSimulatePhysics(true);
}

// Called when the game starts or when spawned
void ACurrencyCrystal::BeginPlay() {
    Super::BeginPlay();

    SpawnTimestamp = FPlatformTime::Seconds();
}

// Called every frame
void ACurrencyCrystal::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

UStaticMeshComponent* ACurrencyCrystal::GetMesh() const { return CrystalMesh; }

float ACurrencyCrystal::GetCurrentAttractionSubjectivity() const {
    const auto timeElapsed = FPlatformTime::Seconds() - SpawnTimestamp;

    // If no curve is set, default to linear behavior
    if (!AttractionResistanceCurve) { return FMath::Clamp(timeElapsed, 0.0f, 1.0f); }

    // Use the curve to determine the attraction resistance
    return AttractionResistanceCurve->GetFloatValue(timeElapsed);
}

void ACurrencyCrystal::UpdateAttraction(const FVector& TargetLocation) {
    // Calculate force direction towards target
    FVector ForceDirection = TargetLocation - GetActorLocation();
    ForceDirection.Normalize();

    // Get attraction strength based on time
    const float AttractionStrength = GetCurrentAttractionSubjectivity();

    // Apply attraction forces
    ApplyAttractionForces(ForceDirection, AttractionStrength);

    // Update and apply rotation
    UpdateRotation();
    ApplyRotationForces(AttractionStrength);
}

void ACurrencyCrystal::UpdateRotation() {
    // Gradually adjust the rotation axis with a small random change
    const FVector RandomAdjustment = FVector(
        FMath::RandRange(-RotationAxisAdjustmentRange, RotationAxisAdjustmentRange),
        FMath::RandRange(-RotationAxisAdjustmentRange, RotationAxisAdjustmentRange),
        FMath::RandRange(-RotationAxisAdjustmentRange, RotationAxisAdjustmentRange)
        );
    CurrentRotationAxis = (CurrentRotationAxis + RandomAdjustment).GetSafeNormal();
}

void ACurrencyCrystal::ApplyAttractionForces(const FVector& ForceDirection,
                                             float AttractionStrength) const {
    // Slerp crystal velocity towards the target
    const FVector CurrentVelocity = CrystalMesh->GetPhysicsLinearVelocity();
    const float CurrentVelMag = CurrentVelocity.Size();

    CrystalMesh->SetPhysicsLinearVelocity(
        FMath::Lerp(CurrentVelocity,
                    ForceDirection * CurrentVelMag,
                    VelocityLerpStrength * AttractionStrength));

    // Add attraction force
    CrystalMesh->AddForce(ForceDirection * AttractionForceStrength * AttractionStrength);

    // Counteract gravity
    const FVector UpwardForce = FVector(0.f, 0.f, UpwardForceStrength * AttractionStrength);
    CrystalMesh->AddForce(UpwardForce);
}

void ACurrencyCrystal::ApplyRotationForces(float AttractionStrength) const {
    const float FinalRotationStrength = RotationForceStrength * AttractionStrength;
    CrystalMesh->AddTorqueInDegrees(
        CurrentRotationAxis * FinalRotationStrength,
        NAME_None,
        false
        );
}


void ACurrencyCrystal::ValidateConfiguration() const {
    // Validate currency amount is positive
    ensureAlways(CurrencyAmount > 0);

    // Validate crystal mesh component is set
    ensureAlways(CrystalMesh->GetStaticMesh() != nullptr);

    // Validate attraction resistance curve is set
    ensureAlways(AttractionResistanceCurve != nullptr);

    // Validate attraction force values are positive
    ensureAlways(AttractionForceStrength > 0.0f);
    ensureAlways(UpwardForceStrength > 0.0f);
    ensureAlways(RotationForceStrength > 0.0f);

    // Validate velocity lerp strength is in valid range
    ensureAlways(VelocityLerpStrength > 0.0f && VelocityLerpStrength <= 1.0f);

    // Validate rotation axis adjustment range is positive
    ensureAlways(RotationAxisAdjustmentRange > 0.0f);
}
