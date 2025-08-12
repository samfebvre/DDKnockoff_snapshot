#include "Currency/CurrencyManagerSettings.h"

void UCurrencyManagerSettings::ValidateConfiguration() const {
    // Validate that at least one currency crystal type is available
    ensureAlwaysMsgf(!AvailableCurrencyCrystals.IsEmpty(),
                     TEXT(
                         "CurrencyManagerSettings: AvailableCurrencyCrystals array is empty! Currency system will not function correctly"
                     ));

    // Validate that all currency crystal classes are valid
    for (int32 Index = 0; Index < AvailableCurrencyCrystals.Num(); Index++) {
        ensureAlwaysMsgf(AvailableCurrencyCrystals[Index] != nullptr,
                         TEXT("CurrencyManagerSettings: AvailableCurrencyCrystals[%d] is null!"),
                         Index);
    }
}
