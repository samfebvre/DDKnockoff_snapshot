#include "ResourceChest/ResourceChest.h"
#include "ResourceChest/ResourceChestAnimInstance.h"
#include "NavModifierComponent.h"
#include "Components/BoxComponent.h"
#include "Entities/EntityData.h"
#include "Entities/EntityManager.h"
#include "Collision/DDCollisionChannels.h"
#include "Core/ManagerHandlerSubsystem.h"
#include "Currency/CurrencySpawner.h"
#include "Currency/CurrencyUtils.h"
#include "Structures/DefensiveStructureNavArea.h"
#include "NavAreas/NavArea_Default.h"


// Sets default values
AResourceChest::AResourceChest() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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

    // Interact collision box - used for player interaction
    InteractCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractCollisionBox"));
    InteractCollisionBox->SetupAttachment(RootComponent);
    InteractCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    InteractCollisionBox->SetCollisionObjectType(DDCollisionChannels::ECC_Interact);
    InteractCollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractCollisionBox->SetCollisionResponseToChannel(DDCollisionChannels::ECC_Interact,
                                                        ECR_Overlap);
    InteractCollisionBox->SetCanEverAffectNavigation(false);

    // Navigation modifier setup
    NavModifierComponent = CreateDefaultSubobject<UNavModifierComponent>(
        TEXT("NavModifierComponent"));
    NavModifierComponent->SetAreaClass(NavAreaClass);

    // Currency Spawner Component
    CurrencySpawner = CreateDefaultSubobject<UCurrencySpawner>(TEXT("CurrencySpawner"));
    CurrencySpawner->SetupAttachment(RootComponent);

    // Create entity data
    EntityData = CreateDefaultSubobject<UEntityData>(TEXT("EntityData"));
}

// Called when the game starts or when spawned
void AResourceChest::BeginPlay() {
    Super::BeginPlay();

    // Handle dependency injection with single call
    EnsureDependenciesInjected();

    // Validate configuration after dependencies are set
    ValidateConfiguration();

    // Register with EntityManager using injected dependency
    EntityManager->RegisterEntity(this);

    AnimInstance = Cast<UResourceChestAnimInstance>(SkeletonMesh->GetAnimInstance());
    ensureAlways(AnimInstance != nullptr);

    NavModifierComponent->SetAreaClass(NavAreaClass);
}

// Called every frame
void AResourceChest::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    if (ChestGrowState == EChestGrowState::Growing ||
        ChestGrowState == EChestGrowState::Shrinking) { UpdateScale(DeltaTime); }
}

void AResourceChest::EndPlay(EEndPlayReason::Type EndPlayReason) {
    // Unregister from EntityManager using injected dependency
    EntityManager->UnregisterEntity(this);
    Super::EndPlay(EndPlayReason);
}

AActor* AResourceChest::GetActor() { return this; }

UEntityData* AResourceChest::GetEntityData() const { return EntityData; }

EEntityType AResourceChest::GetEntityType() const { return EEntityType::Interactable; }

EFaction AResourceChest::GetFaction() const { return EFaction::Player; }

void AResourceChest::Interact() {
    IEntity::Interact();

    if (ChestState != EChestState::Closed) { return; }

    SetChestState(EChestState::Opened);

    // Calculate minimum crystal count based on the selected option
    int32 MinimumCrystalCount = 0;
    switch (MinimumCrystalOption) {
        case EMinimumCrystalOption::Unenforced:
            MinimumCrystalCount = 0;
            break;
        case EMinimumCrystalOption::SpecificMinimum:
            MinimumCrystalCount = SpecificMinimumCrystalCount;
            break;
        case EMinimumCrystalOption::RandomRange:
            MinimumCrystalCount = FMath::RandRange(MinRandomCrystalCount, MaxRandomCrystalCount);
            break;
        case EMinimumCrystalOption::MAX:
            break;
    }

    // Spawn currency using utility function
    UCurrencyUtils::SpawnCurrencyBurstFromActor(
        this,
        CurrencySpawner,
        RewardedCurrencyAmount,
        MinimumCrystalCount,
        CurrencySpawner->GetUpForwardVector(),
        500.0f
        );
}

void AResourceChest::SetChestState(EChestState NewState) {
    ChestState = NewState;
    AnimInstance->CurrentChestState = NewState;

    if (NewState == EChestState::Opened) {
        // Start the shrinking process after a delay
        GetWorld()->GetTimerManager().SetTimer(
            ShrinkTimerHandle,
            this,
            &AResourceChest::StartShrinking,
            ShrinkDelay,
            false
            );
    } else if (NewState == EChestState::Closed) {
        GetWorldTimerManager().ClearTimer(ShrinkTimerHandle);
    }
}

// TODO - this is a bit weird having the anim instance state mirror this. Might want to make it reference this class instead.
void AResourceChest::SetChestGrowState(EChestGrowState NewState) {
    ChestGrowState = NewState;
    AnimInstance->ChestGrowState = NewState;
}

// TODO - this is a bit weird having the anim instance state mirror this. Might want to make it reference this class instead.
void AResourceChest::StartShrinking() { SetChestGrowState(EChestGrowState::Shrinking); }

void AResourceChest::UpdateScale(float DeltaTime) {
    // Calculate how much to shrink this frame
    const float ScaleStep = DeltaTime / ScaleChangeDuration;

    if (EChestGrowState::Growing == ChestGrowState) { ScaleProgress += ScaleStep; } else if (
        EChestGrowState::Shrinking == ChestGrowState) { ScaleProgress -= ScaleStep; }
    // Clamp the scale progress to the range [0, 1]
    ScaleProgress = FMath::Clamp(ScaleProgress, 0.0f, 1.0f);

    // Update scale based on progress
    const FVector NewScale(ScaleProgress, ScaleProgress, ScaleProgress);

    SetActorScale3D(NewScale);

    switch (ChestGrowState) {
        case EChestGrowState::Growing:
            if (ScaleProgress >= 1.0f) { SetChestGrowState(EChestGrowState::FullSize); }
            break;
        case EChestGrowState::Shrinking:
            if (ScaleProgress <= 0.0f) {
                DisableChest();
                SetChestGrowState(EChestGrowState::FullyShrunk);
            }
            break;
        default:
            break;
    }
}

void AResourceChest::DisableChest() const {
    // Disable collision on everything
    PhysicalCollisionMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    InteractCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SkeletonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    // Disable the nav modifier
    NavModifierComponent->SetAreaClass(UNavArea_Default::StaticClass());
}

void AResourceChest::EnableChest() const {
    // Enable collision on everything
    PhysicalCollisionMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    InteractCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SkeletonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    // Enable the nav modifier
    NavModifierComponent->SetAreaClass(NavAreaClass);
}

void AResourceChest::ResetChest() {
    if (ChestState == EChestState::Closed) { return; }

    // Reset the chest state
    SetChestState(EChestState::Closed);
    SetChestGrowState(EChestGrowState::Growing);

    // Enable the chest again
    EnableChest();
}

// IDependencyInjectable interface implementation
bool AResourceChest::HasRequiredDependencies() const { return EntityManager != nullptr; }

void AResourceChest::CollectDependencies() {
    // Get EntityManager from world subsystem (normal gameplay fallback)
    if (!EntityManager) {
        EntityManager = UManagerHandlerSubsystem::GetManager<UEntityManager>(GetWorld());
    }
}

// IConfigurationValidatable interface implementation
void AResourceChest::ValidateConfiguration() const {
    ensureAlways(NavAreaClass != nullptr);

    // Validate timing properties
    ensureAlways(ShrinkDelay > 0.0f);
    ensureAlways(ScaleChangeDuration > 0.0f);

    // Validate currency amount
    ensureAlways(RewardedCurrencyAmount > 0);

    // Validate crystal count settings based on the selected option
    if (MinimumCrystalOption == EMinimumCrystalOption::SpecificMinimum) {
        ensureAlways(SpecificMinimumCrystalCount > 0);
    } else if (MinimumCrystalOption == EMinimumCrystalOption::RandomRange) {
        ensureAlways(MinRandomCrystalCount >= 0);
        ensureAlways(MaxRandomCrystalCount > MinRandomCrystalCount);
    }
}
