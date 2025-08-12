#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerInteractionComponent.generated.h"

class USphereComponent;

/**
 * Result structure for interaction detection operations.
 */
USTRUCT(BlueprintType)
struct FInteractDetectionResult {
    GENERATED_BODY()

    /** Closest interactable actor to the player */
    UPROPERTY(BlueprintReadOnly, Category = "Detection")
    TObjectPtr<AActor> ClosestInteractable = nullptr;

    /** All interactable actors currently in range */
    UPROPERTY(BlueprintReadOnly, Category = "Detection")
    TArray<AActor*> InteractablesInRange;
};

/**
 * Component that handles player interaction with world objects.
 * Detects interactable objects within range and manages interaction events.
 * Provides automatic interaction with all objects in range or targeted interaction.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DDKNOCKOFF_API UPlayerInteractionComponent : public UActorComponent {
    GENERATED_BODY()

public:
    UPlayerInteractionComponent();

    // Component lifecycle
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime,
                               ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // Interaction system

    /**
     * Update the list of interactable objects and find the closest one.
     */
    void UpdateInteractableInformation();

    /**
     * Trigger interaction with all interactable objects currently in range.
     */
    void InteractWithAllInteractablesInRange();

    // State access

    FInteractDetectionResult InteractionDetectionResult;

protected:
    virtual void OnRegister() override;

    /**
     * Process detected actors and populate interaction result data.
     */
    void PopulateInteractableResultData();

    // Components

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Detection")
    TObjectPtr<USphereComponent> InteractionDetectionSphere;

    // Runtime state

    UPROPERTY(Transient)
    TArray<AActor*> ActorsInRange;
};
