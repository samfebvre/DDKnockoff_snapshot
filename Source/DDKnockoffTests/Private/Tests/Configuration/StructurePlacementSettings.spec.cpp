#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Structures/StructurePlacementSettings.h"
#include "Core/DDKnockoffGameSettings.h"
#include "Debug/DebugInformationManager.h"
#include "Tests/Common/BaseSpec.h"

BEGIN_DEFINE_SPEC(FStructurePlacementSettingsAssetValidationSpec,
                  "DDKnockoff.00_Configuration.StructurePlacementSettings",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |
                  EAutomationTestFlags::ProductFilter)

// SPEC_BOILERPLATE_BEGIN
BaseSpec BaseSpec;
// SPEC_BOILERPLATE_END


END_DEFINE_SPEC(FStructurePlacementSettingsAssetValidationSpec)

void FStructurePlacementSettingsAssetValidationSpec::Define() {
    BeforeEach([this] {
        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.SetupBaseSpecEnvironment({UDebugInformationManager::StaticClass()});
        // SPEC_BOILERPLATE_END
    });

    AfterEach([this] {
        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.TeardownBaseSpecEnvironment();
        // SPEC_BOILERPLATE_END
    });
    
    It("should load and validate the default settings asset",
       [this] {
           // Get settings asset path from game settings
            UDDKnockoffGameSettings* GameSettings = const_cast<UDDKnockoffGameSettings*>(
               UDDKnockoffGameSettings::Get());
           TestTrue(TEXT("Game settings should be available"), GameSettings != nullptr);

           if (GameSettings && !GameSettings->StructurePlacementSettingsAsset.IsNull()) {
                UStructurePlacementSettings* TestSettings = Cast<
                   UStructurePlacementSettings>(
                   GameSettings->StructurePlacementSettingsAsset.TryLoad());

               TestTrue(TEXT("Settings asset should load successfully"),
                        TestSettings != nullptr);
               if (TestSettings) { TestSettings->ValidateConfiguration(); }
           }
       });
}
