#pragma once

#include "CoreMinimal.h"
#include "NavAreas/NavArea_Obstacle.h"
#include "DefensiveStructureNavArea.generated.h"

/**
 * Navigation area for defensive structures to control AI pathfinding.
 * Marks defensive structure areas as obstacles for enemy navigation.
 * Ensures AI characters path around defensive structures appropriately.
 */
UCLASS()
class DDKNOCKOFF_API UDefensiveStructureNavArea : public UNavArea_Obstacle {
    GENERATED_BODY()

public:
    UDefensiveStructureNavArea();
};
