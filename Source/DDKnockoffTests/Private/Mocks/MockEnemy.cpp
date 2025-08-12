#include "Mocks/MockEnemy.h"

#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/ManagerHandlerSubsystem.h"
#include "Damage/DamagePayload.h"
#include "Entities/EntityData.h"
#include "Entities/EntityManager.h"
#include "Health/HealthComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AMockEnemy::AMockEnemy() {
    PrimaryActorTick.bCanEverTick = false;

    // Create capsule component for collision and targeting
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    RootComponent = CapsuleComponent;
    CapsuleComponent->SetCapsuleSize(34.0f, 88.0f); // Similar to Character defaults
    CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CapsuleComponent->SetCollisionResponseToAllChannels(ECR_Block);
    CapsuleComponent->SetCollisionObjectType(ECC_Pawn);
    CapsuleComponent->SetHiddenInGame(false);

    // Create health component for damage simulation
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

    // Create entity data
    EntityData = CreateDefaultSubobject<UEntityData>(TEXT("EntityData"));
}

void AMockEnemy::BeginPlay() {
    Super::BeginPlay();

    // Handle dependency injection
    EnsureDependenciesInjected();

    // Register with EntityManager
    EntityManager->RegisterEntity(this);

    // Connect to health component's death event (use Remove+Add pattern for safety)
    if (HealthComponent) {
        HealthComponent->OnReachedZeroHealth.RemoveDynamic(this, &AMockEnemy::OnReachedZeroHealth);
        HealthComponent->OnReachedZeroHealth.AddDynamic(this, &AMockEnemy::OnReachedZeroHealth);
    }
}

void AMockEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason) {
    // Unregister from EntityManager
    EntityManager->UnregisterEntity(this);
    Super::EndPlay(EndPlayReason);
}

void AMockEnemy::TakeDamage(const FDamagePayload& DamagePayload) {
    // Store the damage payload for test verification (always track even if invulnerable)
    LastDamagePayload = DamagePayload;
    SetState(EMockEnemyState::HasBeenDamaged);

    // If invulnerable, don't actually apply damage
    if (HasTestFlag(EMockEnemyFlags::Invulnerable)) {
        UE_LOG(LogTemp, Log, TEXT("MockEnemy is invulnerable - damage ignored"));
        return;
    }

    // Apply damage to health component
    if (HealthComponent) {
        const float HealthBefore = HealthComponent->GetCurrentHealth();
        HealthComponent->TakeDamage(DamagePayload.DamageAmount);
        const float HealthAfter = HealthComponent->GetCurrentHealth();

        UE_LOG(LogTemp,
               Log,
               TEXT("MockEnemy took %.1f damage (Health: %.1f -> %.1f)"),
               DamagePayload.DamageAmount,
               HealthBefore,
               HealthAfter);
    }
}

float AMockEnemy::GetHalfHeight() const {
    if (CapsuleComponent) { return CapsuleComponent->GetScaledCapsuleHalfHeight(); }
    return 88.0f; // Default fallback
}

float AMockEnemy::GetRadius() const {
    return CapsuleComponent ? CapsuleComponent->GetScaledCapsuleRadius() : 34.0f;
    // Default fallback
}

bool AMockEnemy::HasRequiredDependencies() const { return EntityManager != nullptr; }

void AMockEnemy::CollectDependencies() {
    if (!EntityManager) {
        EntityManager = UManagerHandlerSubsystem::GetManager<UEntityManager>(GetWorld());
    }
}

void AMockEnemy::OnReachedZeroHealth() {
    // If immortal, ignore death entirely
    if (HasTestFlag(EMockEnemyFlags::Immortal)) { return; }

    // Normal death behavior - destroy the actor
    Destroy();
}

void AMockEnemy::SetHealth(const float NewHealth) const {
    if (HealthComponent) { HealthComponent->SetCurrentHealthForTesting(NewHealth); }
}

float AMockEnemy::GetCurrentHealth() const {
    if (HealthComponent) { return HealthComponent->GetCurrentHealth(); }
    return 0.0f;
}

bool AMockEnemy::IsDead() const {
    if (HealthComponent) { return HealthComponent->IsDead(); }
    return true; // Safe default - assume dead if no health component
}

void AMockEnemy::SetFaction(EFaction InFaction) { TestFaction = InFaction; }

bool AMockEnemy::WasDamagedBy(const AActor* Actor) const {
    if (!HasState(EMockEnemyState::HasBeenDamaged) || !Actor) { return false; }

    if (LastDamagePayload.DamageInstigator.GetInterface()) {
        return LastDamagePayload.DamageInstigator->GetActor() == Actor;
    }

    return false;
}

bool AMockEnemy::WasDamagedByClass(const TSubclassOf<AActor>& ActorClass) const {
    if (!HasState(EMockEnemyState::HasBeenDamaged) || !ActorClass) { return false; }

    if (LastDamagePayload.DamageInstigator.GetInterface()) {
        if (const AActor* InstigatorActor = LastDamagePayload.DamageInstigator->GetActor()) {
            return InstigatorActor->IsA(ActorClass);
        }
    }

    return false;
}
