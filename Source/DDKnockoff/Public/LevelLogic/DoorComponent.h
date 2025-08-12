#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DoorComponent.generated.h"

// open closed enum
UENUM(BlueprintType)
enum class EDoorState : uint8 {
    Opened,
    Closed,
    Opening,
    Closing
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DDKNOCKOFF_API UDoorComponent : public UActorComponent {
    GENERATED_BODY()

public:
    UDoorComponent();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // The current state of the door
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door")
    EDoorState DoorState = EDoorState::Closed;

    // CurveFloat for opening and closing the door
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    TObjectPtr<UCurveFloat> DoorTweenCurve_Open;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    TObjectPtr<UCurveFloat> DoorTweenCurve_Close;

    float CurrentProgress = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    float TweenDuration = 1.0f; // Duration of the tween in seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    float OpenedAngle = 90.0f; // Angle to open the door to

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    float ClosedAngle = 0.0f; // Angle to close the door to

    void UpdateDoorRotation() const;

    FRotator InitialRotation;

    void ProcessDoorState(float DeltaTime);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    bool FlipDirection;

public:
    // Called every frame
    virtual void TickComponent(float DeltaTime,
                               ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(CallInEditor, Category = "Door")
    void OpenDoor();
    UFUNCTION(CallInEditor, Category = "Door")
    void CloseDoor();
};
