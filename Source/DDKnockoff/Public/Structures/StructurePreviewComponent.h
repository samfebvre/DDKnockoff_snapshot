#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Structures/StructurePlacementEnums.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "StructurePreviewComponent.generated.h"

class UStructurePlacementSettings;
class UStructurePlacementManager;
class USkeletalMeshComponent;

/**
 * Component responsible for managing structure preview materials and visual feedback.
 * This component handles the transition between normal and preview materials,
 * color updates based on placement validity, and restoration of original materials.
 */
UCLASS(BlueprintType,
    ClassGroup = (Custom),
    meta = (BlueprintSpawnableComponent, DisplayName = "Structure Preview Component"))
class DDKNOCKOFF_API UStructurePreviewComponent : public UActorComponent {
    GENERATED_BODY()

public:
    UStructurePreviewComponent();

    // Initialize preview mode - sets up preview material on the target mesh
    UFUNCTION(BlueprintCallable, Category = "Structure Preview")
    void StartPreviewMode(USkeletalMeshComponent* TargetMesh);

    // Update the preview color based on placement validity
    UFUNCTION(BlueprintCallable, Category = "Structure Preview")
    void UpdatePreviewColor(EStructurePlacementValidityState ValidityState,
                            EStructurePlacementInvalidityReason InvalidityReason) const;

    // End preview mode - restore original materials
    UFUNCTION(BlueprintCallable, Category = "Structure Preview")
    void EndPreviewMode();

    // Check if currently in preview mode
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Structure Preview")
    bool IsInPreviewMode() const { return bIsInPreviewMode; }

protected:
    virtual void BeginPlay() override;

private:
    // The mesh component we're managing materials for
    UPROPERTY(Transient)
    USkeletalMeshComponent* TargetMeshComponent;

    // Original materials stored when entering preview mode
    UPROPERTY(Transient)
    TArray<TObjectPtr<UMaterialInterface>> OriginalMaterials;

    // Dynamic material instance for preview
    UPROPERTY(Transient)
    UMaterialInstanceDynamic* PreviewDynamicMaterial;

    // Whether we're currently in preview mode
    UPROPERTY(Transient)
    bool bIsInPreviewMode;

    // Cached reference to placement subsystem
    UPROPERTY(Transient)
    UStructurePlacementManager* PlacementSubsystem;

    // Get the placement settings from the subsystem
    const UStructurePlacementSettings& GetPlacementSettings() const;

    // Apply preview material to all material slots
    void ApplyPreviewMaterial();

    // Restore original materials to all slots
    void RestoreOriginalMaterials();

    // Get the appropriate color for the current validity state
    FLinearColor GetColorForValidityState(EStructurePlacementValidityState ValidityState,
                                          EStructurePlacementInvalidityReason InvalidityReason)
    const;
};
