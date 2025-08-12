#include "Tests/DefenseStructure/DefenseStructureTestHelpers.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Core/DDKnockoffGameSettings.h"
#include "Structures/Ballista/Ballista.h"

UClass* FDefenseStructureTestHelpers::LoadDefenseStructureBlueprintClass(
    const UClass* BaseClass,
    const FString& AssetPath) {
    // Use asset path from settings if not provided
    FString SearchPath = AssetPath;
    if (SearchPath.IsEmpty()) {
        const UDDKnockoffGameSettings* GameSettings = UDDKnockoffGameSettings::Get();
        if (GameSettings) { SearchPath = GameSettings->DefenseStructuresPath; }
    }

    // Fallback to default if still empty
    if (SearchPath.IsEmpty()) { SearchPath = TEXT("/Game/Defense_Structures"); }

    // Load the structure class dynamically from the asset registry
    const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<
        FAssetRegistryModule>("AssetRegistry");
    const IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    TArray<FAssetData> BlueprintAssets;
    AssetRegistry.GetAssetsByPath(FName(*SearchPath), BlueprintAssets, true);

    UClass* LoadedClass = nullptr;
    bool FoundValidClass = false;
    for (const FAssetData& AssetData : BlueprintAssets) {
        FString GeneratedClassPath;
        if (!AssetData.GetTagValue(FBlueprintTags::GeneratedClassPath, GeneratedClassPath)) {
            continue;
        }

        FString ClassObjectPath = FPackageName::ExportTextPathToObjectPath(GeneratedClassPath);
        LoadedClass = LoadObject<UClass>(nullptr, *ClassObjectPath);
        if (!LoadedClass || !LoadedClass->IsChildOf(BaseClass) || LoadedClass == BaseClass) {
            continue;
        }

        FoundValidClass = true;
        break;
    }

    if (!FoundValidClass) { return nullptr; }

    // Log the loaded class
    UE_LOG(LogTemp,
           Log,
           TEXT(
               "FDefenseStructureTestHelpers::LoadDefenseStructureBlueprintClass - Loaded class: %s"
           ),
           *LoadedClass->GetName());
    return LoadedClass;
}

AMockEnemy* FDefenseStructureTestHelpers::PlaceEnemyInDetectionRange(
    UWorld* World,
    const ADefensiveStructure* Structure,
    float DetectionRadius) {
    // Get the enemy's capsule half height from the CDO before spawning
    const AMockEnemy* EnemyCDO = GetDefault<AMockEnemy>();
    const float EnemyHalfHeight = EnemyCDO->GetHalfHeight();

    // Calculate spawn location with proper vertical displacement
    const FVector EnemyLocation = Structure->GetActorLocation() + FVector(
                                      DetectionRadius - 50.0f,
                                      0.0f,
                                      EnemyHalfHeight);

    const FTransform SpawnTransform = FTransform(FRotator::ZeroRotator,
                                                 EnemyLocation,
                                                 FVector::OneVector);
    return World->SpawnActor<AMockEnemy>(AMockEnemy::StaticClass(), SpawnTransform);
}
