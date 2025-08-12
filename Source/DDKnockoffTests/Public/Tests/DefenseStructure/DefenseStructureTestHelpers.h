#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Structures/DefensiveStructure.h"
#include "Mocks/MockEnemy.h"

/**
 * Defense structure specific test helpers
 * Builds on top of generic FTestUtils for defense structure testing patterns
 */
class DDKNOCKOFFTESTS_API FDefenseStructureTestHelpers {
public:
    // Asset loading - defense structure specific
    static UClass* LoadDefenseStructureBlueprintClass(const UClass* BaseClass,
                                                      const FString& AssetPath = FString());

    // Mock enemy management - defense structure context
    static AMockEnemy* PlaceEnemyInDetectionRange(UWorld* World,
                                                  const ADefensiveStructure* Structure,
                                                  float DetectionRadius);
};
