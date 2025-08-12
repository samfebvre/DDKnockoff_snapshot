#include "CoreMinimal.h"
#include "Entities/EntityManager.h"
#include "Tests/Common/BaseSpec.h"
#include "Health/HealthComponent.h"
#include "Mocks/MockEnemy.h"

BEGIN_DEFINE_SPEC(FHealthComponentSpec,
                  "DDKnockoff.Components.HealthComponent",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |
                  EAutomationTestFlags::ProductFilter)

    // SPEC_BOILERPLATE_BEGIN
    BaseSpec BaseSpec;
    // SPEC_BOILERPLATE_END
    
    TObjectPtr<AMockEnemy> TestActor;
    TObjectPtr<UHealthComponent> HealthComponent;

END_DEFINE_SPEC(FHealthComponentSpec)

void FHealthComponentSpec::Define() {
    BeforeEach([this] {
        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.SetupBaseSpecEnvironment({UEntityManager::StaticClass()});
        // SPEC_BOILERPLATE_END

        // Spawn a mock enemy actor (which already has health component setup)
        TestActor = BaseSpec.WorldHelper->GetWorld()->SpawnActor<AMockEnemy>();
        HealthComponent = TestActor->FindComponentByClass<UHealthComponent>();
        
        // Ensure the component exists
        if (!HealthComponent) {
            TestTrue("MockEnemy should have HealthComponent", false);
            return;
        }
    });

    AfterEach([this] {
        // Clear object references
        HealthComponent = nullptr;
        TestActor = nullptr;

        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.TeardownBaseSpecEnvironment();
        // SPEC_BOILERPLATE_END
    });

    Describe("Damage System", [this] {
        It("should reduce health when taking damage", [this] {
            // Arrange
             float InitialHealth = HealthComponent->GetMaxHealth();
             float DamageAmount = 25.0f;
            
            // Act
            HealthComponent->TakeDamage(DamageAmount);
            
            // Assert
            TestEqual("Health should be reduced by damage amount", 
                     HealthComponent->GetCurrentHealth(), InitialHealth - DamageAmount);
        });

        It("should clamp health to zero minimum", [this] {
            // Arrange - damage more than max health
             float ExcessiveDamage = HealthComponent->GetMaxHealth() + 50.0f;
            
            // Act
            HealthComponent->TakeDamage(ExcessiveDamage);
            
            // Assert
            TestEqual("Health should not go below zero", HealthComponent->GetCurrentHealth(), 0.0f);
        });

        It("should ignore negative damage", [this] {
            // Arrange
             float InitialHealth = HealthComponent->GetCurrentHealth();
            
            // Act
            HealthComponent->TakeDamage(-10.0f);
            
            // Assert
            TestEqual("Negative damage should be ignored", HealthComponent->GetCurrentHealth(), InitialHealth);
        });

        It("should ignore zero damage", [this] {
            // Arrange
             float InitialHealth = HealthComponent->GetCurrentHealth();
            
            // Act
            HealthComponent->TakeDamage(0.0f);
            
            // Assert
            TestEqual("Zero damage should be ignored", HealthComponent->GetCurrentHealth(), InitialHealth);
        });
    });

    Describe("Death System", [this] {
        It("should report dead when health is zero", [this] {
            // Arrange - set health to zero
            HealthComponent->TakeDamage(HealthComponent->GetMaxHealth());
            
            // Act & Assert
            TestTrue("Should report dead when health is zero", HealthComponent->IsDead());
        });

        It("should report alive when health is above zero", [this] {
            // Arrange - ensure health is above zero
            HealthComponent->TakeDamage(HealthComponent->GetMaxHealth() - 10.0f);
            
            // Act & Assert
            TestFalse("Should report alive when health is above zero", HealthComponent->IsDead());
        });
    });

    Describe("Initialization", [this] {
        It("should initialize CurrentHealth to MaxHealth", [this] {
            // This should be true after BeginPlay, which happens during actor spawn
            TestEqual("CurrentHealth should equal MaxHealth after initialization", 
                     HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth());
        });
    });
}