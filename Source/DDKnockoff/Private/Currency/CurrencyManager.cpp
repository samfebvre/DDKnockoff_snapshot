#include "Currency/CurrencyManager.h"
#include "Core/ConfigurationValidatable.h"

UCurrencyManager::UCurrencyManager() {
    // Constructor implementation
}

void UCurrencyManager::Initialize() {}

void UCurrencyManager::Deinitialize() {}

void UCurrencyManager::SetSettings(UCurrencyManagerSettings* NewSettings) {
    Settings = NewSettings;

    // Validate settings if they implement IConfigurationValidatable
    if (Settings && Cast<IConfigurationValidatable>(Settings)) {
        Cast<IConfigurationValidatable>(Settings)->ValidateConfiguration();
    }

    SortCrystalTypes();
}

void UCurrencyManager::SortCrystalTypes() const {
    if (!Settings) { return; }

    // Sort crystal types by currency amount (highest to lowest)
    Settings->AvailableCurrencyCrystals.Sort(
        [](const TSubclassOf<ACurrencyCrystal>& A, const TSubclassOf<ACurrencyCrystal>& B) {
            const ACurrencyCrystal* DefaultA = A.GetDefaultObject();
            const ACurrencyCrystal* DefaultB = B.GetDefaultObject();
            return DefaultA->GetCurrencyAmount() > DefaultB->GetCurrencyAmount();
        });
}

// TODO - refactor this, the performance is horrible
TArray<FCurrencySpawnInfo> UCurrencyManager::CalculateCurrencySpawnInfo(
    int32 TotalCurrencyAmount,
    int32 MinimumCrystalCount) const {
    TArray<FCurrencySpawnInfo> Result;

    if (TotalCurrencyAmount <= 0 || !Settings || Settings->AvailableCurrencyCrystals.Num() == 0) {
        return Result;
    }

    int32 RemainingAmount = TotalCurrencyAmount;

    // First pass: Try to use the highest value crystals first
    for (const TSubclassOf<ACurrencyCrystal>& CrystalClass : Settings->AvailableCurrencyCrystals) {
        const ACurrencyCrystal* DefaultCrystal = CrystalClass.GetDefaultObject();
        if (!DefaultCrystal || DefaultCrystal->GetCurrencyAmount() <= 0) { continue; }

        // Calculate how many of this crystal type we can use
        const int32 Count = RemainingAmount / DefaultCrystal->GetCurrencyAmount();

        if (Count > 0) {
            FCurrencySpawnInfo SpawnInfo;
            SpawnInfo.CrystalClass = CrystalClass;
            SpawnInfo.Count = Count;
            Result.Add(SpawnInfo);

            RemainingAmount -= Count * DefaultCrystal->GetCurrencyAmount();
        }

        if (RemainingAmount == 0) { break; }
    }

    // If we still have remaining amount, use the smallest crystal type to cover the difference
    if (RemainingAmount > 0 && Settings->AvailableCurrencyCrystals.Num() > 0) {
        const TSubclassOf<ACurrencyCrystal>& SmallestCrystalClass = Settings->
            AvailableCurrencyCrystals.Last();
        const ACurrencyCrystal* DefaultCrystal = SmallestCrystalClass.GetDefaultObject();

        if (DefaultCrystal && DefaultCrystal->GetCurrencyAmount() > 0) {
            FCurrencySpawnInfo SpawnInfo;
            SpawnInfo.CrystalClass = SmallestCrystalClass;
            SpawnInfo.Count = 1;
            Result.Add(SpawnInfo);
        }
    }

    // Calculate total number of crystals
    int32 TotalCrystals = 0;
    for (const FCurrencySpawnInfo& SpawnInfo : Result) { TotalCrystals += SpawnInfo.Count; }

    // If we need more crystals, break down larger ones into smaller ones
    if (TotalCrystals < MinimumCrystalCount && Settings->AvailableCurrencyCrystals.Num() > 1) {
        // Start from the largest crystal (index 0)
        for (int32 i = 0; i < Settings->AvailableCurrencyCrystals.Num() - 1; i++) {
            const TSubclassOf<ACurrencyCrystal>& CurrentCrystalClass = Settings->
                AvailableCurrencyCrystals[i];
            const TSubclassOf<ACurrencyCrystal>& SmallerCrystalClass = Settings->
                AvailableCurrencyCrystals[i + 1];

            const ACurrencyCrystal* CurrentCrystal = CurrentCrystalClass.GetDefaultObject();
            const ACurrencyCrystal* SmallerCrystal = SmallerCrystalClass.GetDefaultObject();

            if (!CurrentCrystal || !SmallerCrystal) { continue; }

            // Find the current crystal in our results
            for (int32 j = 0; j < Result.Num(); j++) {
                if (Result[j].CrystalClass == CurrentCrystalClass && Result[j].Count > 0) {
                    // Calculate how many smaller crystals we can get from one larger crystal
                    const int32 SmallerCrystalsPerLarge =
                        CurrentCrystal->GetCurrencyAmount() / SmallerCrystal->GetCurrencyAmount();

                    // Calculate how many large crystals we need to break down
                    const int32 CrystalsNeeded = MinimumCrystalCount - TotalCrystals;
                    int32 LargeCrystalsToBreak = FMath::CeilToInt(
                        static_cast<float>(CrystalsNeeded) / SmallerCrystalsPerLarge);
                    LargeCrystalsToBreak = FMath::Min(LargeCrystalsToBreak, Result[j].Count);

                    if (LargeCrystalsToBreak > 0) {
                        // Reduce the count of larger crystals
                        Result[j].Count -= LargeCrystalsToBreak;

                        // Add the smaller crystals
                        bool FoundSmaller = false;
                        for (FCurrencySpawnInfo& SpawnInfo : Result) {
                            if (SpawnInfo.CrystalClass == SmallerCrystalClass) {
                                SpawnInfo.Count += LargeCrystalsToBreak * SmallerCrystalsPerLarge;
                                FoundSmaller = true;
                                break;
                            }
                        }

                        if (!FoundSmaller) {
                            FCurrencySpawnInfo NewSpawnInfo;
                            NewSpawnInfo.CrystalClass = SmallerCrystalClass;
                            NewSpawnInfo.Count = LargeCrystalsToBreak * SmallerCrystalsPerLarge;
                            Result.Add(NewSpawnInfo);
                        }

                        TotalCrystals += LargeCrystalsToBreak * SmallerCrystalsPerLarge -
                            LargeCrystalsToBreak;

                        if (TotalCrystals >= MinimumCrystalCount) { break; }
                    }
                }
            }

            if (TotalCrystals >= MinimumCrystalCount) { break; }
        }
    }

    // Clean up any entries with zero count
    Result.RemoveAll([](const FCurrencySpawnInfo& SpawnInfo) { return SpawnInfo.Count <= 0; });

    return Result;
}
