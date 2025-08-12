#include "Structures/StructurePreviewComponent.h"
#include "Structures/StructurePlacementManager.h"
#include "Structures/StructurePlacementSettings.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/ManagerHandlerSubsystem.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"

UStructurePreviewComponent::UStructurePreviewComponent() {
    PrimaryComponentTick.bCanEverTick = false;
    bIsInPreviewMode = false;
    TargetMeshComponent = nullptr;
    PreviewDynamicMaterial = nullptr;
    PlacementSubsystem = nullptr;
}

void UStructurePreviewComponent::BeginPlay() {
    Super::BeginPlay();

    // Cache reference to placement subsystem
    PlacementSubsystem = UManagerHandlerSubsystem::GetManager<UStructurePlacementManager>(
        GetWorld());
    ensureAlways(PlacementSubsystem);
}

void UStructurePreviewComponent::StartPreviewMode(USkeletalMeshComponent* TargetMesh) {
    if (!TargetMesh) {
        UE_LOG(LogTemp,
               Warning,
               TEXT("StructurePreviewComponent: Cannot start preview mode with null target mesh"));
        return;
    }

    if (bIsInPreviewMode) {
        UE_LOG(LogTemp, Warning, TEXT("StructurePreviewComponent: Already in preview mode"));
        return;
    }

    TargetMeshComponent = TargetMesh;
    bIsInPreviewMode = true;

    // Store original materials
    OriginalMaterials.Empty();
    for (int32 MaterialIndex = 0; MaterialIndex < TargetMeshComponent->GetMaterials().Num();
         MaterialIndex++) {
        OriginalMaterials.Add(TargetMeshComponent->GetMaterial(MaterialIndex));
    }

    // Apply preview material
    ApplyPreviewMaterial();

    UE_LOG(LogTemp, Log, TEXT("StructurePreviewComponent: Started preview mode"));
}

void UStructurePreviewComponent::UpdatePreviewColor(EStructurePlacementValidityState ValidityState,
                                                    EStructurePlacementInvalidityReason
                                                    InvalidityReason) const {
    if (!bIsInPreviewMode || !PreviewDynamicMaterial) { return; }

    const FLinearColor ColorToUse = GetColorForValidityState(ValidityState, InvalidityReason);
    PreviewDynamicMaterial->SetVectorParameterValue(FName("Color"), ColorToUse);
}

void UStructurePreviewComponent::EndPreviewMode() {
    if (!bIsInPreviewMode) { return; }

    RestoreOriginalMaterials();

    bIsInPreviewMode = false;
    TargetMeshComponent = nullptr;
    PreviewDynamicMaterial = nullptr;
    OriginalMaterials.Empty();

    UE_LOG(LogTemp, Log, TEXT("StructurePreviewComponent: Ended preview mode"));
}

const UStructurePlacementSettings& UStructurePreviewComponent::GetPlacementSettings() const {
    return PlacementSubsystem->GetPlacementSettings();
}

void UStructurePreviewComponent::ApplyPreviewMaterial() {
    if (!TargetMeshComponent) { return; }

    const UMaterialInterface& PreviewMaterial = PlacementSubsystem->GetPreviewMaterial();

    // Create dynamic material instance (Create requires non-const pointer)
    PreviewDynamicMaterial = UMaterialInstanceDynamic::Create(
        const_cast<UMaterialInterface*>(&PreviewMaterial),
        this);
    if (!PreviewDynamicMaterial) {
        UE_LOG(LogTemp,
               Error,
               TEXT("StructurePreviewComponent: Failed to create dynamic material instance"));
        return;
    }

    // Apply to all material slots
    for (int32 MaterialIndex = 0; MaterialIndex < TargetMeshComponent->GetMaterials().Num();
         MaterialIndex++) {
        TargetMeshComponent->SetMaterial(MaterialIndex, PreviewDynamicMaterial);
    }
}

void UStructurePreviewComponent::RestoreOriginalMaterials() {
    if (!TargetMeshComponent || OriginalMaterials.Num() == 0) { return; }

    // Restore original materials
    for (int32 MaterialIndex = 0; MaterialIndex < OriginalMaterials.Num() && MaterialIndex <
                                  TargetMeshComponent->GetMaterials().Num(); MaterialIndex++) {
        TargetMeshComponent->SetMaterial(MaterialIndex, OriginalMaterials[MaterialIndex]);
    }
}

FLinearColor UStructurePreviewComponent::GetColorForValidityState(
    EStructurePlacementValidityState ValidityState,
    EStructurePlacementInvalidityReason InvalidityReason) const {
    const UStructurePlacementSettings& Settings = GetPlacementSettings();

    if (ValidityState == EStructurePlacementValidityState::Valid) {
        return Settings.ValidPlacementColor;
    }
    // Check if the invalidity reason is ONLY distance
    if (InvalidityReason == EStructurePlacementInvalidityReason::Distance) {
        return Settings.PartiallyInvalidPlacementColor;
    }
    return Settings.InvalidPlacementColor;
}
