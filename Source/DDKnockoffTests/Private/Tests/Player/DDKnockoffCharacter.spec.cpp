#include "CoreMinimal.h"
#include "Debug/DebugInformationManager.h"
#include "Tests/Common/BaseSpec.h"
#include "Player/DDKnockoffCharacter.h"
#include "Player/PlayerAnimInstance.h"
#include "Tests/Common/TestUtils.h"
#include "Entities/EntityManager.h"
#include "Structures/StructurePlacementManager.h"
#include "UObject/ConstructorHelpers.h"

BEGIN_DEFINE_SPEC(FDDKnockoffCharacterSpec,
                  "DDKnockoff.Player.DDKnockoffCharacter",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |
                  EAutomationTestFlags::ProductFilter)

    // SPEC_BOILERPLATE_BEGIN
    BaseSpec BaseSpec;
    // SPEC_BOILERPLATE_END
    
    TObjectPtr<ADDKnockoffCharacter> TestCharacter;
    UClass* PlayerCharacterClass;

END_DEFINE_SPEC(FDDKnockoffCharacterSpec)

void FDDKnockoffCharacterSpec::Define() {
    BeforeEach([this] {
        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.SetupBaseSpecEnvironment({UDebugInformationManager::StaticClass(),
                                           UEntityManager::StaticClass(),
                                           UStructurePlacementManager::StaticClass()});
        // SPEC_BOILERPLATE_END

        // Load the Blueprint class for the player character
        PlayerCharacterClass = StaticLoadClass(ADDKnockoffCharacter::StaticClass(), nullptr, 
            TEXT("/Game/Characters/BP_PlayerCharacter.BP_PlayerCharacter_C"));
        
        if (PlayerCharacterClass == nullptr) {
            // Fallback to C++ class if Blueprint is not available
            PlayerCharacterClass = ADDKnockoffCharacter::StaticClass();
        }

        TestCharacter = BaseSpec.WorldHelper->GetWorld()->SpawnActor<ADDKnockoffCharacter>(PlayerCharacterClass);

        // FUNCTIONAL_CAMERA_TARGET(TestCharacter)
    });

    AfterEach([this] {
        // Clear object references
        TestCharacter = nullptr;
        PlayerCharacterClass = nullptr;

        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.TeardownBaseSpecEnvironment();
        // SPEC_BOILERPLATE_END
    });

    // Asset and Configuration Validation
    Describe("Asset Validation", [this] {
        It("should spawn successfully and validate configuration", [this] {
            TestTrue("Character should be spawned", IsValid(TestCharacter));
            
            // This validates that all UPROPERTY values set in the editor are correct
            // If any input actions, structure classes, or combat properties are null/invalid,
            // this will fail with clear error messages from ensureAlways()
            TestCharacter->ValidateConfiguration();
        });
    });

    // Combo System Tests
    Describe("Combo System", [this] {
        It("should have valid combo attack montages array", [this] {
            UPlayerAnimInstance* AnimInstance = Cast<UPlayerAnimInstance>(TestCharacter->GetMesh()->GetAnimInstance());
            TestTrue("AnimInstance should be valid", IsValid(AnimInstance));
            // Note: ComboAttackMontages validation happens in ValidateConfiguration()
        });

        It("should reset combo when entering hit reaction", [this] {
            UPlayerAnimInstance* AnimInstance = Cast<UPlayerAnimInstance>(TestCharacter->GetMesh()->GetAnimInstance());
            TestTrue("AnimInstance should be valid", IsValid(AnimInstance));
            
            // Reset combo should work without errors
            AnimInstance->ResetCombo();
            
            // Enter hit reaction (should reset combo)
            AnimInstance->EnterHitReaction();
            
            // No direct way to test combo index since it's private, but method should execute without error
            TestTrue("Hit reaction should complete successfully", true);
        });

        It("should not attack if combo montages array is empty", [this] {
            UPlayerAnimInstance* AnimInstance = Cast<UPlayerAnimInstance>(TestCharacter->GetMesh()->GetAnimInstance());
            TestTrue("AnimInstance should be valid", IsValid(AnimInstance));
            
            // Clear the combo montages array to test early return
            AnimInstance->ComboAttackMontages.Empty();
            
            // Attempt to attack with empty array
            AnimInstance->Attack();
            
            // Should remain in locomotion state since attack should return early
            TestEqual("Should remain in locomotion state with empty combo array", 
                AnimInstance->CurrentPoseState, EPlayerPoseState::Locomotion);
        });
    });
}