#include "Tests/DefenseStructure/DefensiveStructure_CommonTests.h"
#include "Entities/Entity.h"
#include "Structures/DefensiveStructure.h"

namespace DefensiveStructureTests_CommonTests
{

	void TestEntityProperties(FAutomationTestBase* Test, ADefensiveStructure* DefensiveStructure)
	{
		Test->TestEqual("Faction is Player", DefensiveStructure->GetFaction(), EFaction::Player);
		Test->TestEqual("EntityType is Structure_Defense", DefensiveStructure->GetEntityType(), EEntityType::Structure_Defense);
	    Test->TestEqual( "Actor reference is correct", DefensiveStructure->GetActor(), static_cast<AActor*> (DefensiveStructure) );
	}

    void TestTargetability(FAutomationTestBase* Test, ADefensiveStructure* DefensiveStructure) {
        // Test default targetable state
        Test->TestTrue("Entity is targetable when not previewing",
                       DefensiveStructure->IsCurrentlyTargetable());

	    // Test targetability during previewing state
        DefensiveStructure->OnStartedPreviewing();
        Test->TestFalse("Entity is not targetable when previewing",
                        DefensiveStructure->IsCurrentlyTargetable());

	    // Test targetability after placement
        DefensiveStructure->OnPlaced();
        Test->TestTrue("Entity is targetable after placement",
                       DefensiveStructure->IsCurrentlyTargetable());
    }

	void TestEntityPropertiesForFunctionalTest(AFunctionalTest* Test, ADefensiveStructure* DefensiveStructure)
	{
		Test->AssertEqual_Int(static_cast<int32>(DefensiveStructure->GetFaction()), static_cast<int32>(EFaction::Player), TEXT("Faction is Player"));
		Test->AssertEqual_Int(static_cast<int32>(DefensiveStructure->GetEntityType()), static_cast<int32>(EEntityType::Structure_Defense), TEXT("EntityType is Structure_Defense"));
		Test->AssertTrue(DefensiveStructure->GetActor() == static_cast<AActor*>(DefensiveStructure), TEXT("Actor reference is correct"));
	}

	void TestTargetabilityForFunctionalTest(AFunctionalTest* Test, ADefensiveStructure* DefensiveStructure)
	{
		// Test default targetable state
		Test->AssertTrue(DefensiveStructure->IsCurrentlyTargetable(), TEXT("Entity is targetable when not previewing"));

		// Test targetability during previewing state
		DefensiveStructure->OnStartedPreviewing();
		Test->AssertFalse(DefensiveStructure->IsCurrentlyTargetable(), TEXT("Entity is not targetable when previewing"));

		// Test targetability after placement
		DefensiveStructure->OnPlaced();
		Test->AssertTrue(DefensiveStructure->IsCurrentlyTargetable(), TEXT("Entity is targetable after placement"));
	}
}
