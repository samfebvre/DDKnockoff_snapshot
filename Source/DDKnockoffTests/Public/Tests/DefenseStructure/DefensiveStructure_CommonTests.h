#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "FunctionalTest.h"
#include "Structures/DefensiveStructure.h"

class IEntity;
class UMockEntityManager;

namespace DefensiveStructureTests_CommonTests {
void TestEntityProperties(FAutomationTestBase* Test, ADefensiveStructure* DefensiveStructure);
void TestTargetability(FAutomationTestBase* Test, ADefensiveStructure* DefensiveStructure);

void TestEntityPropertiesForFunctionalTest(AFunctionalTest* Test,
                                           ADefensiveStructure* DefensiveStructure);
void TestTargetabilityForFunctionalTest(AFunctionalTest* Test,
                                        ADefensiveStructure* DefensiveStructure);
}
