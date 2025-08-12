#include "CoreMinimal.h"
#include "LevelLogic/WaveManager.h"
#include "Misc/AutomationTest.h"
#include "LevelLogic/WaveManagerSettings.h"
#include "Core/DDKnockoffGameSettings.h"

BEGIN_DEFINE_SPEC(FWaveManagerSettingsAssetValidationSpec,
                  "DDKnockoff.00_Configuration.WaveManagerSettings",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |
                  EAutomationTestFlags::ProductFilter)

    // Get settings path from game settings configuration
    UWaveManagerSettings* TestSettings;

END_DEFINE_SPEC(FWaveManagerSettingsAssetValidationSpec)

void FWaveManagerSettingsAssetValidationSpec::Define() {
    Describe("Asset Validation",
             [this] {
                 It("should load and validate the default settings asset", [this]()
                 {
                     // Get settings asset path from game settings
                     const UDDKnockoffGameSettings* GameSettings = UDDKnockoffGameSettings::Get();
                     TestTrue("Game settings should be available", GameSettings != nullptr);
                     
                     if (GameSettings && !GameSettings->WaveManagerSettingsAsset.IsNull())
                     {
                         TestSettings = Cast<UWaveManagerSettings>(
                             GameSettings->WaveManagerSettingsAsset.TryLoad());

                         // Check test results
                         TestTrue("Settings asset should load successfully", TestSettings != nullptr);
                         if (TestSettings) TestSettings->ValidateConfiguration();
                     }
                 });
             });
}