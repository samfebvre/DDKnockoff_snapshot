#include "Structures/StructurePlacementSettings.h"

UStructurePlacementSettings::UStructurePlacementSettings() {
    // Set default values
    ValidPlacementColor = FLinearColor::Green;
    PartiallyInvalidPlacementColor = FLinearColor::Yellow;
    InvalidPlacementColor = FLinearColor::Red;
    MaxPlacementDistance = 1000.0f;
    PositioningPhaseRotationSpeed = 15.0f;
    RotationPhaseRotationSpeed = 5.0f;

    // Set default preview material path - this would be set to an actual material asset in the editor
    PreviewMaterial = nullptr;
}

void UStructurePlacementSettings::ValidateConfiguration() const {
    ensureAlways(!PreviewMaterial.IsNull());
    ensureAlways(MaxPlacementDistance > 0.0f);
    ensureAlways(PositioningPhaseRotationSpeed > 0.0f);
    ensureAlways(RotationPhaseRotationSpeed > 0.0f);
}
