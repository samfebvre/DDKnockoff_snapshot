#include "../../Public/LevelLogic/DoorComponent.h"


// Sets default values
UDoorComponent::UDoorComponent()
    : InitialRotation() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    // PrimaryActorTick.bCanEverTick = true;

    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

// Called when the game starts or when spawned
void UDoorComponent::BeginPlay() {
    Super::BeginPlay();

    InitialRotation = GetOwner()->GetActorRotation();
}

void UDoorComponent::UpdateDoorRotation() const {
    FRotator DesiredRotation = InitialRotation;
    float TargetOffset = 0.0f;

    auto GetLerpAlpha = [](const UCurveFloat* Curve, float Progress) -> float {
        return Curve ? Curve->GetFloatValue(Progress) : Progress;
    };

    switch (DoorState) {
        case EDoorState::Opened:
            TargetOffset = OpenedAngle;
            break;

        case EDoorState::Closed:
            TargetOffset = ClosedAngle;
            break;

        case EDoorState::Opening:
            TargetOffset = FMath::Lerp(ClosedAngle,
                                       OpenedAngle,
                                       GetLerpAlpha(DoorTweenCurve_Open, CurrentProgress));
            break;

        case EDoorState::Closing:
            TargetOffset = FMath::Lerp(ClosedAngle,
                                       OpenedAngle,
                                       GetLerpAlpha(DoorTweenCurve_Close, CurrentProgress));
            break;

        default:
            break;
    }

    DesiredRotation.Yaw += TargetOffset * (FlipDirection ? -1.0f : 1.0f);
    GetOwner()->SetActorRotation(DesiredRotation);
}


void UDoorComponent::ProcessDoorState(const float DeltaTime) {
    switch (DoorState) {
        case EDoorState::Opened:
            break;
        case EDoorState::Closed:
            break;
        case EDoorState::Opening: {
            const auto AdditionalProgress = DeltaTime / TweenDuration;
            CurrentProgress += AdditionalProgress;

            if (CurrentProgress >= 1.0f) { DoorState = EDoorState::Opened; }
            break;
        }

        case EDoorState::Closing: {
            const auto AdditionalProgress = DeltaTime / TweenDuration;
            CurrentProgress -= AdditionalProgress;

            if (CurrentProgress <= 0.0f) { DoorState = EDoorState::Closed; }
            break;
        }
    }

    CurrentProgress = FMath::Clamp(CurrentProgress, 0.0f, 1.0f);
}

void UDoorComponent::TickComponent(float DeltaTime,
                                   ELevelTick TickType,
                                   FActorComponentTickFunction* ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    ProcessDoorState(DeltaTime);
    UpdateDoorRotation();
}


void UDoorComponent::OpenDoor() {
    if (DoorState == EDoorState::Opened || DoorState == EDoorState::Opening) {
        return; // Already closed or in the process of opening
    }
    CurrentProgress = 0;
    DoorState = EDoorState::Opening;
}

void UDoorComponent::CloseDoor() {
    if (DoorState == EDoorState::Closed || DoorState == EDoorState::Closing) {
        return; // Already closed or in the process of opening
    }
    CurrentProgress = 1;
    DoorState = EDoorState::Closing;
}
