#include "CoreMinimal.h"
#include "Tests/Common/BaseSpec.h"
#include "Currency/CurrencyManager.h"
#include "Currency/CurrencyManagerSettings.h"

BEGIN_DEFINE_SPEC(FCurrencyManagerSpec,
                  "DDKnockoff.Currency.CurrencyManager",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |
                  EAutomationTestFlags::ProductFilter)

    // SPEC_BOILERPLATE_BEGIN
    BaseSpec BaseSpec;
    // SPEC_BOILERPLATE_END
    
    TObjectPtr<UCurrencyManager> CurrencyManager;

END_DEFINE_SPEC(FCurrencyManagerSpec)

void FCurrencyManagerSpec::Define() {
    BeforeEach([this] {
        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.SetupBaseSpecEnvironment({UCurrencyManager::StaticClass()});
        // SPEC_BOILERPLATE_END

        UManagerHandlerSubsystem* ManagerHandler = BaseSpec.WorldHelper->GetWorld()->GetSubsystem<UManagerHandlerSubsystem>();
        CurrencyManager =ManagerHandler->GetManager<UCurrencyManager>();
        TestTrue("CurrencyManager should be available", CurrencyManager != nullptr);
    });

    AfterEach([this] {
        CurrencyManager = nullptr;

        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.TeardownBaseSpecEnvironment();
        // SPEC_BOILERPLATE_END
    });


    Describe("Basic Currency Calculation", [this] {
        It("should return empty array for invalid inputs", [this] {
            // Act
            TArray<FCurrencySpawnInfo> ZeroResult = CurrencyManager->CalculateCurrencySpawnInfo(0, 1);
            TArray<FCurrencySpawnInfo> NegativeResult = CurrencyManager->CalculateCurrencySpawnInfo(-10, 1);

            // Assert
            TestEqual("Zero amount should return empty array", ZeroResult.Num(), 0);
            TestEqual("Negative amount should return empty array", NegativeResult.Num(), 0);
        });
    });
}