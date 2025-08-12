#include "Structures/Ballista/BallistaAmmo.h"

#include "Collision/DDCollisionChannels.h"
#include "Core/ManagerHandlerSubsystem.h"
#include "Damage/DamageUtils.h"
#include "Entities/EntityData.h"
#include "Entities/EntityManager.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Const requirement disabled since it breaks compilation when using dynamic multicast delegates
// ReSharper disable once CppMemberFunctionMayBeConst
void ABallistaAmmo::OnPhysicalHit(UPrimitiveComponent* HitComponent,
                                  AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp,
                                  FVector NormalImpulse,
                                  const FHitResult& Hit) {
    VisualMesh->SetSimulatePhysics(true);
    ProjectileMovementComponent->StopMovementImmediately();
    ProjectileMovementComponent->Deactivate();
    HitboxMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    VisualMesh->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
    VisualMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    VisualMesh->SetCollisionResponseToChannel(DDCollisionChannels::ECC_EnemyPawn, ECR_Block);
}

void ABallistaAmmo::OnHitboxOverlap(UPrimitiveComponent* OverlappedComponent,
                                    AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp,
                                    int32 OtherBodyIndex,
                                    bool bFromSweep,
                                    const FHitResult& SweepResult) {
    // If the other actor is ourself, ignore it
    // Apply damage with knockback using centralized utility
    UDamageUtils::ApplyDamage(OtherActor, this, 50.0f, 100.0f, EDDDamageType::Ranged);
}

// Sets default values
ABallistaAmmo::ABallistaAmmo() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Create entity data
    EntityData = CreateDefaultSubobject<UEntityData>(TEXT("EntityData"));

    // Set up root
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Set up visual mesh
    VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    VisualMesh->SetupAttachment(RootComponent);
    VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    VisualMesh->SetCollisionObjectType(ECC_PhysicsBody);
    VisualMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    VisualMesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    VisualMesh->SetSimulatePhysics(false);
    VisualMesh->SetCanEverAffectNavigation(false);

    // Set up hitbox component
    HitboxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HitboxComponent"));
    HitboxMesh->SetupAttachment(VisualMesh);
    HitboxMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HitboxMesh->SetCollisionObjectType(DDCollisionChannels::ECC_Hitbox);
    HitboxMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    HitboxMesh->SetCollisionResponseToChannel(DDCollisionChannels::ECC_Hurtbox, ECR_Overlap);
    HitboxMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    HitboxMesh->SetCollisionResponseToChannel(DDCollisionChannels::ECC_EnemyPawn, ECR_Overlap);
    HitboxMesh->SetGenerateOverlapEvents(true);
    HitboxMesh->SetVisibility(false);
    HitboxMesh->SetCanEverAffectNavigation(false);

    // Set up projectile movement component
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(
        TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->SetUpdatedComponent(RootComponent);
    ProjectileMovementComponent->InitialSpeed = 0.0f;
    ProjectileMovementComponent->MaxSpeed = 1000.0f;
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->Velocity = FVector::ZeroVector;
}

// Called when the game starts or when spawned
void ABallistaAmmo::BeginPlay() {
    Super::BeginPlay();

    // Handle dependency injection with single call
    EnsureDependenciesInjected();

    // Validate configuration after dependencies are set
    ValidateConfiguration();

    // Register with EntityManager using injected dependency
    EntityManager->RegisterEntity(this);

    VisualMesh->OnComponentHit.RemoveDynamic(this, &ABallistaAmmo::OnPhysicalHit);
    VisualMesh->OnComponentHit.AddDynamic(this, &ABallistaAmmo::OnPhysicalHit);
    HitboxMesh->OnComponentBeginOverlap.RemoveDynamic(this, &ABallistaAmmo::OnHitboxOverlap);
    HitboxMesh->OnComponentBeginOverlap.AddDynamic(this, &ABallistaAmmo::OnHitboxOverlap);
}

// Called every frame
void ABallistaAmmo::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void ABallistaAmmo::Fire() {
    SetLifeSpan(5.0f);

    // Set the initial velocity of the projectile
    const FVector ForwardVector = GetActorForwardVector();
    ProjectileMovementComponent->Velocity = ForwardVector * ProjectileSpeed;
    ProjectileMovementComponent->Activate();
    VisualMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    HitboxMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

EFaction ABallistaAmmo::GetFaction() const { return EFaction::Player; }

UEntityData* ABallistaAmmo::GetEntityData() const { return EntityData; }
EEntityType ABallistaAmmo::GetEntityType() const { return EEntityType::Projectile; }

AActor* ABallistaAmmo::GetActor() { return this; }

void ABallistaAmmo::EndPlay(const EEndPlayReason::Type EndPlayReason) {
    // Unregister from EntityManager using injected dependency
    EntityManager->UnregisterEntity(this);
    Super::EndPlay(EndPlayReason);
}

// IDependencyInjectable interface implementation
bool ABallistaAmmo::HasRequiredDependencies() const { return EntityManager != nullptr; }

void ABallistaAmmo::CollectDependencies() {
    if (EntityManager == nullptr) {
        EntityManager = UManagerHandlerSubsystem::GetManager<UEntityManager>(GetWorld());
    }
}

// IConfigurationValidatable interface implementation
void ABallistaAmmo::ValidateConfiguration() const {
    // Validate that ProjectileSpeed is positive
    ensureAlways(ProjectileSpeed > 0.0f);
}
