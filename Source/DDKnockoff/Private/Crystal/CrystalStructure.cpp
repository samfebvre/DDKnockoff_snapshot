#include "Crystal/CrystalStructure.h"

#include "Crystal/CrystalAnimInstance.h"
#include "NavModifierComponent.h"
#include "Damage/DamagePayload.h"
#include "Health/HealthComponent.h"
#include "Animation/AnimInstance.h"
#include "Collision/DDCollisionChannels.h"
#include "Entities/EntityData.h"
#include "Entities/EntityManager.h"
#include "Structures/DefensiveStructureNavArea.h"
#include "Core/DDKnockoffGameSettings.h"
#include "Core/ManagerHandlerSubsystem.h"
#include "UObject/ConstructorHelpers.h"


// Sets default values
ACrystalStructure::ACrystalStructure() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    PrimaryActorTick.bCanEverTick = true;

    // Set up root
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Skeleton mesh - the visual representation of the structure
    SkeletonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletonMesh"));
    SkeletonMesh->SetupAttachment(RootComponent);
    SkeletonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SkeletonMesh->SetVisibility(true);

    // Physical collision mesh - the physical representation of the structure
    PhysicalCollisionMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("PhysicalCollisionMesh"));
    PhysicalCollisionMesh->SetupAttachment(RootComponent);
    PhysicalCollisionMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    PhysicalCollisionMesh->SetCollisionObjectType(ECC_WorldDynamic);
    PhysicalCollisionMesh->SetCollisionResponseToAllChannels(ECR_Block);
    PhysicalCollisionMesh->SetVisibility(false);
    PhysicalCollisionMesh->SetCanEverAffectNavigation(false);

    // Hurtbox mesh - the hurtbox for the structure
    HurtboxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HurtboxMesh"));
    HurtboxMesh->SetupAttachment(RootComponent);
    HurtboxMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    HurtboxMesh->SetCollisionObjectType(DDCollisionChannels::ECC_Hurtbox);
    HurtboxMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    HurtboxMesh->SetCollisionResponseToChannel(DDCollisionChannels::ECC_Hitbox, ECR_Overlap);
    HurtboxMesh->SetCollisionResponseToChannel(DDCollisionChannels::ECC_Trigger, ECR_Overlap);
    HurtboxMesh->SetVisibility(false);
    HurtboxMesh->SetGenerateOverlapEvents(true);
    HurtboxMesh->SetCanEverAffectNavigation(false);

    // Health setup
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    HealthComponent->SetCanEverAffectNavigation(false);

    // Navigation modifier setup
    NavModifierComponent = CreateDefaultSubobject<UNavModifierComponent>(
        TEXT("NavModifierComponent"));

    // Load nav area class from game settings
    const UDDKnockoffGameSettings* GameSettings = UDDKnockoffGameSettings::Get();
    if (GameSettings && !GameSettings->DefenseNavAreaClass.IsEmpty()) {
        static ConstructorHelpers::FClassFinder<UDefensiveStructureNavArea> NavAreaClassFinder(
            *GameSettings->DefenseNavAreaClass);
        UClass* NavAreaClass = NavAreaClassFinder.Class;
        if (NavAreaClass) { NavModifierComponent->SetAreaClass(NavAreaClass); }
    }

    // Create entity data
    EntityData = CreateDefaultSubobject<UEntityData>(TEXT("EntityData"));
}

void ACrystalStructure::TakeDamage(const FDamagePayload& DamagePayload) {
    IEntity::TakeDamage(DamagePayload);

    HealthComponent->TakeDamage(DamagePayload.DamageAmount);
}

void ACrystalStructure::OnDeath() { Destroy(); }

// Called when the game starts or when spawned
void ACrystalStructure::BeginPlay() {
    Super::BeginPlay();

    // Handle dependency injection with single call
    EnsureDependenciesInjected();

    // Validate configuration
    ValidateConfiguration();

    // Register with EntityManager using injected dependency
    EntityManager->RegisterEntity(this);

    AnimInstance = Cast<UCrystalAnimInstance>(SkeletonMesh->GetAnimInstance());
    ensureAlways(AnimInstance != nullptr);

    HealthComponent->OnReachedZeroHealth.RemoveDynamic(this, &ACrystalStructure::OnDeath);
    HealthComponent->OnReachedZeroHealth.AddDynamic(this, &ACrystalStructure::OnDeath);
}

// Called every frame
void ACrystalStructure::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    AnimInstance->UpdateAnimationProperties(DeltaTime);
}

void ACrystalStructure::EndPlay(EEndPlayReason::Type EndPlayReason) {
    // Unregister from EntityManager using injected dependency
    EntityManager->UnregisterEntity(this);

    Super::EndPlay(EndPlayReason);
}

EFaction ACrystalStructure::GetFaction() const { return EFaction::Player; }
AActor* ACrystalStructure::GetActor() { return this; }
UEntityData* ACrystalStructure::GetEntityData() const { return EntityData; }
EEntityType ACrystalStructure::GetEntityType() const { return EEntityType::Structure_Crystal; }

// IDependencyInjectable interface implementation
bool ACrystalStructure::HasRequiredDependencies() const { return EntityManager != nullptr; }

void ACrystalStructure::CollectDependencies() {
    // Get EntityManager from world subsystem (normal gameplay fallback)
    if (!EntityManager) {
        EntityManager = UManagerHandlerSubsystem::GetManager<UEntityManager>(GetWorld());
    }
}

// IConfigurationValidatable interface implementation
void ACrystalStructure::ValidateConfiguration() const {
    // Validate components
    ensureAlways(SkeletonMesh != nullptr);
    ensureAlways(PhysicalCollisionMesh != nullptr);
    ensureAlways(HurtboxMesh != nullptr);
    ensureAlways(HealthComponent != nullptr);
    ensureAlways(NavModifierComponent != nullptr);
    ensureAlways(EntityData != nullptr);

    // Validate mesh and animation setup
    ensureAlways(SkeletonMesh->GetSkeletalMeshAsset() != nullptr);
    ensureAlways(SkeletonMesh->GetAnimClass() != nullptr);
}
