#include "CoreMinimal.h"
#include "Tests/Common/BaseSpec.h"
#include "Damage/DamageUtils.h"
#include "Entities/EntityManager.h"
#include "Mocks/MockEnemy.h"

BEGIN_DEFINE_SPEC(FDamageUtilsSpec,
                  "DDKnockoff.Damage.DamageUtils",
                  EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |
                  EAutomationTestFlags::ProductFilter)

    // SPEC_BOILERPLATE_BEGIN
    BaseSpec BaseSpec;
    // SPEC_BOILERPLATE_END
    
    TObjectPtr<AMockEnemy> PlayerEntity;
    TObjectPtr<AMockEnemy> EnemyEntity;
    TObjectPtr<AMockEnemy> AllyEntity;

END_DEFINE_SPEC(FDamageUtilsSpec)

void FDamageUtilsSpec::Define() {
    BeforeEach([this] {
        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.SetupBaseSpecEnvironment({UEntityManager::StaticClass()});
        // SPEC_BOILERPLATE_END

        // Create test entities with different factions
        auto PlayerSpawnTransform = FTransform::Identity;
        PlayerEntity = BaseSpec.WorldHelper->GetWorld()->SpawnActorDeferred<AMockEnemy>( AMockEnemy::StaticClass(), PlayerSpawnTransform);
        PlayerEntity->SetFaction(EFaction::Player);
        PlayerEntity->FinishSpawning(PlayerSpawnTransform);

        auto EnemySpawnTransform = FTransform::Identity;
        EnemyEntity = BaseSpec.WorldHelper->GetWorld()->SpawnActorDeferred<AMockEnemy>( AMockEnemy::StaticClass(), EnemySpawnTransform);
        EnemyEntity->SetFaction(EFaction::Enemy);
        EnemyEntity->FinishSpawning(EnemySpawnTransform);

        auto AllySpawnTransform = FTransform::Identity;
        AllyEntity = BaseSpec.WorldHelper->GetWorld()->SpawnActorDeferred<AMockEnemy>( AMockEnemy::StaticClass(), AllySpawnTransform);
        AllyEntity->SetFaction(EFaction::Player);
        AllyEntity->FinishSpawning(AllySpawnTransform);
    });

    AfterEach([this] {
        PlayerEntity = nullptr;
        EnemyEntity = nullptr;
        AllyEntity = nullptr;

        // SPEC_BOILERPLATE_BEGIN
        BaseSpec.TeardownBaseSpecEnvironment();
        // SPEC_BOILERPLATE_END
    });

    Describe("Damage Validation", [this] {
        It("should allow damage between different factions", [this] {
            // Act
            bool bCanDamage = UDamageUtils::CanDamageTarget(EnemyEntity, PlayerEntity);

            // Assert
            TestTrue("Should allow damage between different factions", bCanDamage);
        });

        It("should prevent damage between same factions", [this] {
            // Act
            bool bCanDamage = UDamageUtils::CanDamageTarget(AllyEntity, PlayerEntity);

            // Assert
            TestFalse("Should prevent damage between same factions", bCanDamage);
        });

        It("should prevent self-damage", [this] {
            // Act
            bool bCanDamage = UDamageUtils::CanDamageTarget(PlayerEntity, PlayerEntity);

            // Assert
            TestFalse("Should prevent self-damage", bCanDamage);
        });

        It("should handle null targets gracefully", [this] {
            // Act
            bool bCanDamage = UDamageUtils::CanDamageTarget(nullptr, PlayerEntity);

            // Assert
            TestFalse("Should handle null targets", bCanDamage);
        });

        It("should handle null instigators gracefully", [this] {
            // Act
            bool bCanDamage = UDamageUtils::CanDamageTarget(EnemyEntity, nullptr);

            // Assert
            TestFalse("Should handle null instigators", bCanDamage);
        });
    });

    Describe("Faction Checking", [this] {
        It("should identify different factions correctly", [this] {
            // Act
            bool bDifferentFactions = UDamageUtils::AreDifferentFactions(PlayerEntity, EnemyEntity);

            // Assert
            TestTrue("Should identify different factions", bDifferentFactions);
        });

        It("should identify same factions correctly", [this] {
            // Act
            bool bDifferentFactions = UDamageUtils::AreDifferentFactions(PlayerEntity, AllyEntity);

            // Assert
            TestFalse("Should identify same factions", bDifferentFactions);
        });
    });

    Describe("Knockback Direction Calculation", [this] {
        It("should calculate direction from source to target", [this] {
            // Arrange
            PlayerEntity->SetActorLocation(FVector(0, 0, 0));
            EnemyEntity->SetActorLocation(FVector(100, 0, 0));

            // Act
            FVector Direction = UDamageUtils::CalculateKnockbackDirection(EnemyEntity, PlayerEntity);

            // Assert
            TestEqual("Should point in positive X direction", static_cast<float>(Direction.X), 1.0f, 0.01f);
            TestEqual("Should have zero Y component", static_cast<float>(Direction.Y), 0.0f, 0.01f);
            TestEqual("Should have upward Z component", static_cast<float>(Direction.Z), 0.2f, 0.01f);
        });

        It("should handle custom upward component", [this] {
            // Arrange
            PlayerEntity->SetActorLocation(FVector(0, 0, 0));
            EnemyEntity->SetActorLocation(FVector(100, 0, 0));

            // Act
            FVector Direction = UDamageUtils::CalculateKnockbackDirection(EnemyEntity, PlayerEntity, 0.5f);

            // Assert
            TestEqual("Should use custom upward component", static_cast<float>(Direction.Z), 0.5f, 0.01f);
        });
    });

    Describe("Damage Payload Creation", [this] {
        It("should create damage payload with correct values", [this] {
            // Act
            FDamagePayload Payload = UDamageUtils::CreateDamagePayload(25.0f, PlayerEntity, 50.0f, EDDDamageType::Melee);

            // Assert
            TestEqual("Should set damage amount", Payload.DamageAmount, 25.0f);
            TestEqual("Should set damage instigator", Payload.DamageInstigator.GetObject(), Cast<UObject>(PlayerEntity));
            TestEqual("Should set knockback strength", Payload.KnockbackStrength, 50.0f);
            TestEqual("Should set damage type", static_cast<uint8>(Payload.DamageType), static_cast<uint8>(EDDDamageType::Melee));
        });
        
        It("should create damage payload without knockback", [this] {
            // Act
            FDamagePayload Payload = UDamageUtils::CreateDamagePayload(15.0f, PlayerEntity, 0.0f, EDDDamageType::Ranged);

            // Assert
            TestEqual("Should set damage amount", Payload.DamageAmount, 15.0f);
            TestEqual("Should set zero knockback", Payload.KnockbackStrength, 0.0f);
        });
    });

    Describe("Damage Application", [this] {
        It("should apply damage without knockback to valid targets", [this] {
            // Arrange
            float InitialHealth = EnemyEntity->GetCurrentHealth();

            // Act
            bool bDamageApplied = UDamageUtils::ApplyDamage(EnemyEntity, PlayerEntity, 10.0f, 0.0f, EDDDamageType::Ranged);

            // Assert
            TestTrue("Should successfully apply damage", bDamageApplied);
            TestEqual("Should reduce enemy health", EnemyEntity->GetCurrentHealth(), InitialHealth - 10.0f);
        });

        It("should not apply damage to same faction", [this] {
            // Arrange
            float InitialHealth = AllyEntity->GetCurrentHealth();

            // Act
            bool bDamageApplied = UDamageUtils::ApplyDamage(AllyEntity, PlayerEntity, 10.0f, 0.0f, EDDDamageType::Ranged);

            // Assert
            TestFalse("Should not apply damage to same faction", bDamageApplied);
            TestEqual("Health should remain unchanged", AllyEntity->GetCurrentHealth(), InitialHealth);
        });
        
        It("should apply damage with knockback to valid targets", [this] {
            // Arrange
            float InitialHealth = EnemyEntity->GetCurrentHealth();
            PlayerEntity->SetActorLocation(FVector(0, 0, 0));
            EnemyEntity->SetActorLocation(FVector(100, 0, 0));

            // Act
            bool bDamageApplied = UDamageUtils::ApplyDamage(EnemyEntity, PlayerEntity, 15.0f, 50.0f, EDDDamageType::Melee);

            // Assert
            TestTrue("Should successfully apply damage with knockback", bDamageApplied);
            TestEqual("Should reduce enemy health", EnemyEntity->GetCurrentHealth(), InitialHealth - 15.0f);
            // Note: Knockback testing would require checking velocity or position changes in MockEnemy
        });
    });

}