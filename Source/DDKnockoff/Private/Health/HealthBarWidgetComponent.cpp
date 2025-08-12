#include "Health/HealthBarWidgetComponent.h"

#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UHealthBarWidgetComponent::UHealthBarWidgetComponent() {
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}


// Called when the game starts
void UHealthBarWidgetComponent::BeginPlay() { Super::BeginPlay(); }


// Called every frame
void UHealthBarWidgetComponent::TickComponent(float DeltaTime,
                                              ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction) {
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    const APlayerCameraManager* CamMgr = UGameplayStatics::GetPlayerCameraManager(this, 0);
    if (CamMgr) {
        const auto camRotation = CamMgr->GetCameraRotation();
        const auto WidgetRot = FRotator(camRotation.Pitch + 180, camRotation.Yaw, 0);
        SetWorldRotation(WidgetRot);

        // Get widget location
        const FVector WidgetLocation = GetComponentLocation();
        // Get camera location
        const FVector CameraLocation = CamMgr->GetCameraLocation();

        // Calculate the distance between the widget and the camera
        const float Distance = FVector::Dist(WidgetLocation, CameraLocation);
        // Set the scale based on the distance
        const float scale = 1000.0f / Distance;

        // Set the scale of the widget
        GetWidget()->SetRenderScale(FVector2D(scale, scale));
    }
}
