#include "Core/DDKnockoffGameSettings.h"

UDDKnockoffGameSettings::UDDKnockoffGameSettings() {
    // Initialize default values if needed
}

const UDDKnockoffGameSettings* UDDKnockoffGameSettings::Get() {
    return GetDefault<UDDKnockoffGameSettings>();
}

void UDDKnockoffGameSettings::ValidateConfiguration() const {
    // Validate that all required settings are properly configured
    ensureAlwaysMsgf(!DefaultPlayerCharacterClass.IsEmpty(),
                     TEXT("DefaultPlayerCharacterClass is not set in DDKnockoffGameSettings"));
    ensureAlwaysMsgf(!DebugInformationSettingsAsset.IsNull(),
                     TEXT("DebugInformationSettingsAsset is not set in DDKnockoffGameSettings"));
    ensureAlwaysMsgf(!WaveManagerSettingsAsset.IsNull(),
                     TEXT("WaveManagerSettingsAsset is not set in DDKnockoffGameSettings"));
    ensureAlwaysMsgf(!StructurePlacementSettingsAsset.IsNull(),
                     TEXT("StructurePlacementSettingsAsset is not set in DDKnockoffGameSettings"));
    ensureAlwaysMsgf(!DefenseNavAreaClass.IsEmpty(),
                     TEXT("DefenseNavAreaClass is not set in DDKnockoffGameSettings"));
    ensureAlwaysMsgf(!BallistaAmmoClass.IsEmpty(),
                     TEXT("BallistaAmmoClass is not set in DDKnockoffGameSettings"));
    ensureAlwaysMsgf(!HealthBarWidgetClass.IsEmpty(),
                     TEXT("HealthBarWidgetClass is not set in DDKnockoffGameSettings"));
    ensureAlwaysMsgf(!DefenseStructuresPath.IsEmpty(),
                     TEXT("DefenseStructuresPath is not set in DDKnockoffGameSettings"));
}
