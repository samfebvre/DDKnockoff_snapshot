#include "CoreMinimal.h"
#include "Core/ManagerHandlerSubsystem.h"
#include "Tests/Common/BaseSpec.h"
#include "Structures/SpikeBlockade/SpikeBlockade.h"
#include "Tests/Common/TestUtils.h"
#include "Tests/DefenseStructure/DefenseStructureTestHelpers.h"
#include "Tests/DefenseStructure/DefensiveStructure_CommonTests.h"
#include "Mocks/MockEnemy.h"
#include "Damage/DDDamageType.h"
#include "Damage/DamagePayload.h"
#include "Debug/DebugInformationManager.h"
#include "Entities/EntityManager.h"
#include "Structures/StructurePlacementManager.h"

BEGIN_DEFINE_SPEC(FSpikeBlockadeSpec,
                  "DDKnockoff.Structures.SpikeBlockade",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |
                  EAutomationTestFlags::ProductFilter)

    // SPEC_BOILERPLATE_BEGIN
    BaseSpec BaseSpec;
    // SPEC_BOILERPLATE_END
    
    TObjectPtr<ASpikeBlockade> TestSpikeBlockade;

END_DEFINE_SPEC(FSpikeBlockadeSpec)

void FSpikeBlockadeSpec::Define() {
    BeforeEach([this] {
        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.SetupBaseSpecEnvironment({UDebugInformationManager::StaticClass(),
                                           UEntityManager::StaticClass(),
                                           UStructurePlacementManager::StaticClass()});
        // SPEC_BOILERPLATE_END

        UClass* LoadedClass = FDefenseStructureTestHelpers::LoadDefenseStructureBlueprintClass(ASpikeBlockade::StaticClass());
        if (LoadedClass == nullptr) {
            TestTrue("SpikeBlockade class should be loaded successfully", false);
            return;
        }

        TestSpikeBlockade = BaseSpec.WorldHelper->GetWorld()->SpawnActor<ASpikeBlockade>(LoadedClass);

        // FUNCTIONAL_CAMERA_TARGET(TestSpikeBlockade)
    });

    AfterEach([this] {
        // Clear object references
        TestSpikeBlockade = nullptr;

        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.TeardownBaseSpecEnvironment();
        // SPEC_BOILERPLATE_END
    });

    // Asset and Spawning
    Describe("Asset Validation", [this] {
        It("should spawn successfully from blueprint asset", [this] {
            // The test succeeds if BeforeEach completes without issues
            // TestSpikeBlockade being valid means the asset loaded and spawned correctly
            TestTrue("SpikeBlockade should be spawned", IsValid(TestSpikeBlockade));
        });
    });

    // Common DefensiveStructure Tests
    Describe("Entity Properties", [this] {
        It("should have correct entity properties", [this] {
            
            // Test defensive structure specific properties
            DefensiveStructureTests_CommonTests::TestEntityProperties(this, TestSpikeBlockade);
        });
    });

    Describe("Targeting Behavior", [this] {
        It("should handle its targetability correctly", [this] {
            DefensiveStructureTests_CommonTests::TestTargetability(this, TestSpikeBlockade);
        });
    });

    // Spike-specific passive damage reflection
    Describe("Damage Reflection System", [this] {
        It("should reflect damage when hit by melee attacks", [this] {
            // Arrange - create enemy and set it up to attack spike blockade
            const FTransform EnemyTransform = FTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector::OneVector);
            AMockEnemy* MockEnemy = BaseSpec.WorldHelper->GetWorld()->SpawnActor<AMockEnemy>(AMockEnemy::StaticClass(), EnemyTransform);
            MockEnemy->SetTestFlag(EMockEnemyFlags::Immortal); // Prevent death during test
             float InitialEnemyHealth = MockEnemy->GetCurrentHealth();
            
            // Act - enemy attacks spike blockade with melee damage
            FDamagePayload MeleeDamagePayload;
            MeleeDamagePayload.DamageAmount = 25.0f;
            MeleeDamagePayload.DamageType = EDDDamageType::Melee;
            MeleeDamagePayload.DamageInstigator = MockEnemy;
            
            TestSpikeBlockade->TakeDamage(MeleeDamagePayload);
            
            // Assert - enemy should have taken reflected damage
            TestTrue("Enemy should take reflected damage from spike blockade", 
                    MockEnemy->GetCurrentHealth() < InitialEnemyHealth);
            TestTrue("Reflected damage should be 50 points", 
                    FMath::IsNearlyEqual(MockEnemy->GetCurrentHealth(), InitialEnemyHealth - 50.0f, 0.1f));
        });

        It("should not reflect damage from non-melee attacks", [this] {
            // Arrange - create enemy 
            const FTransform EnemyTransform = FTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector::OneVector);
            AMockEnemy* MockEnemy = BaseSpec.WorldHelper->GetWorld()->SpawnActor<AMockEnemy>(AMockEnemy::StaticClass(), EnemyTransform);
            MockEnemy->SetTestFlag(EMockEnemyFlags::Immortal);
            float InitialEnemyHealth = MockEnemy->GetCurrentHealth();
            
            // Act - enemy attacks spike blockade with projectile damage
            FDamagePayload ProjectileDamagePayload;
            ProjectileDamagePayload.DamageAmount = 25.0f;
            ProjectileDamagePayload.DamageType = EDDDamageType::Ranged;
            ProjectileDamagePayload.DamageInstigator = MockEnemy;
            
            TestSpikeBlockade->TakeDamage(ProjectileDamagePayload);
            
            // Assert - enemy should not take any reflected damage
            TestTrue("Enemy should not take reflected damage from non-melee attacks", 
                    FMath::IsNearlyEqual(MockEnemy->GetCurrentHealth(), InitialEnemyHealth, 0.1f));
        });
    });

}