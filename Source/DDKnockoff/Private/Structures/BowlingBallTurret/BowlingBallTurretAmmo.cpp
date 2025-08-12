#include "Structures/BowlingBallTurret/BowlingBallTurretAmmo.h"

#include "Collision/DDCollisionChannels.h"
#include "Core/ManagerHandlerSubsystem.h"
#include "Damage/DamageUtils.h"
#include "Entities/EntityData.h"
#include "Entities/EntityManager.h"

void ABowlingBallTurretAmmo::OnPhysicalHit(UPrimitiveComponent* HitComponent,
                                           AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp,
                                           FVector NormalImpulse,
                                           const FHitResult& Hit) {
    // Apply damage with knockback using centralized utility
    UDamageUtils::ApplyDamage(OtherActor, this, 5.0f, 500.0f, EDDDamageType::Ranged);
}

void ABowlingBallTurretAmmo::DisableCollisionWithOwner(ABowlingBallTurret* own) {
    if (own) {
        // Mesh->actor
    }
}

// Sets default values
ABowlingBallTurretAmmo::ABowlingBallTurretAmmo() {
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Set up root
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Set up visual mesh
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    Mesh->SetupAttachment(RootComponent);
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Mesh->SetCollisionObjectType(ECC_PhysicsBody);
    Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    Mesh->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    Mesh->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
    Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    Mesh->SetCollisionResponseToChannel(DDCollisionChannels::ECC_EnemyPawn, ECR_Block);
    Mesh->SetSimulatePhysics(true);
    Mesh->SetCanEverAffectNavigation(false);

    // Create entity data
    EntityData = CreateDefaultSubobject<UEntityData>(TEXT("EntityData"));
}

// Called when the game starts or when spawned
void ABowlingBallTurretAmmo::BeginPlay() {
    Super::BeginPlay();

    // Handle dependency injection with single call
    EnsureDependenciesInjected();

    // Validate configuration after dependencies are set
    ValidateConfiguration();

    // Register with EntityManager using injected dependency
    EntityManager->RegisterEntity(this);

    Mesh->OnComponentHit.RemoveDynamic(this, &ABowlingBallTurretAmmo::OnPhysicalHit);
    Mesh->OnComponentHit.AddDynamic(this, &ABowlingBallTurretAmmo::OnPhysicalHit);
}

// Called every frame
void ABowlingBallTurretAmmo::Tick(float DeltaTime) { Super::Tick(DeltaTime); }

void ABowlingBallTurretAmmo::Fire() {
    SetLifeSpan(5.0f);

    // Set the initial velocity of the projectile
    const FVector ForwardVector = GetActorForwardVector();
    const FVector Velocity = ForwardVector * ProjectileSpeed;
    Mesh->SetPhysicsLinearVelocity(Velocity);
}

EFaction ABowlingBallTurretAmmo::GetFaction() const { return EFaction::Player; }

UEntityData* ABowlingBallTurretAmmo::GetEntityData() const { return EntityData; }
EEntityType ABowlingBallTurretAmmo::GetEntityType() const { return EEntityType::Projectile; }

AActor* ABowlingBallTurretAmmo::GetActor() { return this; }

void ABowlingBallTurretAmmo::EndPlay(const EEndPlayReason::Type EndPlayReason) {
    // Unregister from EntityManager using injected dependency
    EntityManager->UnregisterEntity(this);
    Super::EndPlay(EndPlayReason);
}

// IDependencyInjectable interface implementation
bool ABowlingBallTurretAmmo::HasRequiredDependencies() const { return EntityManager != nullptr; }

void ABowlingBallTurretAmmo::CollectDependencies() {
    // Get EntityManager from world subsystem (normal gameplay fallback)
    if (EntityManager == nullptr) {
        EntityManager = UManagerHandlerSubsystem::GetManager<UEntityManager>(GetWorld());
    }
}

// IConfigurationValidatable interface implementation
void ABowlingBallTurretAmmo::ValidateConfiguration() const {
    // Validate that ProjectileSpeed is positive
    ensureAlways(ProjectileSpeed > 0.0f);
}
