#include "Structures/DefensiveStructure.h"

#include "Structures/DefensiveStructureNavArea.h"
#include "Structures/StructurePreviewComponent.h"
#include "NavModifierComponent.h"
#include "Damage/DamagePayload.h"
#include "Health/HealthComponent.h"
#include "Animation/AnimInstance.h"
#include "Collision/DDCollisionChannels.h"
#include "Entities/EntityData.h"
#include "Entities/EntityManager.h"
#include "NavAreas/NavArea_Default.h"
#include "Core/DDKnockoffGameSettings.h"
#include "Core/ManagerHandlerSubsystem.h"
#include "Utils/CollisionUtils.h"
#include "UObject/ConstructorHelpers.h"

void ADefensiveStructure::TakeDamage(const FDamagePayload& DamagePayload) {
    HealthComponent->TakeDamage(DamagePayload.DamageAmount);
}

void ADefensiveStructure::OnDeath() { Destroy(); }
EDefensiveStructureState ADefensiveStructure::GetCurrentState() const { return CurrentState; }

// Sets default values
ADefensiveStructure::ADefensiveStructure() {
    PrimaryActorTick.bCanEverTick = true;

    // Set up root
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent->SetMobility(EComponentMobility::Movable);

    // Skeleton mesh - the visual representation of the structure
    SkeletonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletonMesh"));
    SkeletonMesh->SetupAttachment(RootComponent);
    SkeletonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SkeletonMesh->SetVisibility(true);

    // Physical collision mesh - the physical representation of the structure
    PhysicalCollisionMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("PhysicalCollisionMesh"));
    PhysicalCollisionMesh->SetupAttachment(RootComponent);
    UCollisionUtils::SetupStructurePhysicalCollision(PhysicalCollisionMesh);

    // Preview collision mesh - the mesh used for placement validity checks.
    PreviewCollisionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewStaticMesh"));
    PreviewCollisionMesh->SetupAttachment(RootComponent);
    PreviewCollisionMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PreviewCollisionMesh->SetCollisionObjectType(DDCollisionChannels::ECC_PreviewStructure);
    PreviewCollisionMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    PreviewCollisionMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
    PreviewCollisionMesh->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
    PreviewCollisionMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    PreviewCollisionMesh->SetCollisionResponseToChannel(DDCollisionChannels::ECC_EnemyPawn,
                                                        ECR_Overlap);
    PreviewCollisionMesh->SetVisibility(false);
    PreviewCollisionMesh->SetCanEverAffectNavigation(false);

    // Hurtbox mesh - the hurtbox for the structure
    HurtboxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HurtboxMesh"));
    HurtboxMesh->SetupAttachment(RootComponent);
    UCollisionUtils::SetupHurtbox(HurtboxMesh);

    // Health setup
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

    // Navigation modifier setup
    NavModifierComponent = CreateDefaultSubobject<UNavModifierComponent>(
        TEXT("NavModifierComponent"));
    NavModifierComponent->SetAreaClass(UNavArea_Default::StaticClass());

    // Structure preview component setup
    StructurePreviewComponent = CreateDefaultSubobject<UStructurePreviewComponent>(
        TEXT("StructurePreviewComponent"));

    // Load nav area class from game settings
    if (NavAreaClass == nullptr) {
        const UDDKnockoffGameSettings* GameSettings = UDDKnockoffGameSettings::Get();
        if (GameSettings && !GameSettings->DefenseNavAreaClass.IsEmpty()) {
            static ConstructorHelpers::FClassFinder<UDefensiveStructureNavArea> NavAreaClassFinder(
                *GameSettings->DefenseNavAreaClass);
            NavAreaClass = NavAreaClassFinder.Class;
        }
    }

    // Create entity data
    EntityData = CreateDefaultSubobject<UEntityData>(TEXT("EntityData"));
}

// IDependencyInjectable interface implementation
bool ADefensiveStructure::HasRequiredDependencies() const { return EntityManager != nullptr; }

void ADefensiveStructure::CollectDependencies() {
    // Only collect EntityManager if not already set (e.g., from manual injection)
    if (!EntityManager) {
        EntityManager = UManagerHandlerSubsystem::GetManager<UEntityManager>(GetWorld());
    }
}

void ADefensiveStructure::ValidateConfiguration() const {
    ensureAlways(NavAreaClass != nullptr);
    ensureAlways(SkeletonMesh != nullptr);
    ensureAlways(SkeletonMesh->GetSkeletalMeshAsset() != nullptr);
}

USkeletalMeshComponent* ADefensiveStructure::GetSkeletonMesh() const { return SkeletonMesh; }

UStaticMeshComponent* ADefensiveStructure::GetPreviewCollisionMesh() const {
    return PreviewCollisionMesh;
}

void ADefensiveStructure::OnStartedPreviewing() {
    StructurePlacementState = EStructurePlacementState::Previewing;

    PhysicalCollisionMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PreviewCollisionMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // Start preview mode using the component
    StructurePreviewComponent->StartPreviewMode(SkeletonMesh);
}

void ADefensiveStructure::OnPlaced() {
    StructurePlacementState = EStructurePlacementState::NotPreviewing;

    PhysicalCollisionMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    PreviewCollisionMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    NavModifierComponent->SetAreaClass(NavAreaClass);

    // End preview mode using the component
    StructurePreviewComponent->EndPreviewMode();
}

void ADefensiveStructure::UpdatePreviewMaterialColor(EStructurePlacementValidityState ValidityState,
                                                     EStructurePlacementInvalidityReason
                                                     InvalidityReason) const {
    StructurePreviewComponent->UpdatePreviewColor(ValidityState, InvalidityReason);
}

// Called when the game starts or when spawned
void ADefensiveStructure::BeginPlay() {
    Super::BeginPlay();

    BaseAnimInstance = SkeletonMesh->GetAnimInstance();

    // Handle dependency injection with single call
    EnsureDependenciesInjected();

    // Validate editor-configured properties
    ValidateConfiguration();

    // Register with EntityManager using injected dependency
    EntityManager->RegisterEntity(this);

    HealthComponent->OnReachedZeroHealth.RemoveDynamic(this, &ADefensiveStructure::OnDeath);
    HealthComponent->OnReachedZeroHealth.AddDynamic(this, &ADefensiveStructure::OnDeath);
}

// Called every frame
void ADefensiveStructure::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void ADefensiveStructure::EndPlay(const EEndPlayReason::Type EndPlayReason) {
    // Unregister from EntityManager using injected dependency
    EntityManager->UnregisterEntity(this);

    Super::EndPlay(EndPlayReason);
}

EFaction ADefensiveStructure::GetFaction() const { return EFaction::Player; }
AActor* ADefensiveStructure::GetActor() { return this; }
UEntityData* ADefensiveStructure::GetEntityData() const { return EntityData; }
EEntityType ADefensiveStructure::GetEntityType() const { return EEntityType::Structure_Defense; }

bool ADefensiveStructure::IsCurrentlyTargetable() const {
    return StructurePlacementState == EStructurePlacementState::NotPreviewing;
}


bool ADefensiveStructure::IsAttackOffCooldown() const {
    if (const UWorld* World = GetWorld()) {
        const float CurrentTime = World->GetTimeSeconds();
        return CurrentTime - LastAttackTime >= AttackCooldownSeconds;
    }
    return false;
}

void ADefensiveStructure::RecordAttack() {
    if (const UWorld* World = GetWorld()) { LastAttackTime = World->GetTimeSeconds(); }
}
