#include "Structures/BounceBlockade/BouncerBlockade.h"

#include "Structures/BounceBlockade/BouncerBlockadeAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Damage/DamageUtils.h"
#include "Utils/CollisionUtils.h"
#include "Structures/Components/EnemyDetectionComponent.h"

ABouncerBlockade::ABouncerBlockade() {
    PrimaryActorTick.bCanEverTick = true;

    // Create hitbox components
    HitboxComponent1 = CreateHitboxComponent(TEXT("HitboxComponent1"), TEXT("hitbox1"));
    HitboxComponent2 = CreateHitboxComponent(TEXT("HitboxComponent2"), TEXT("hitbox2"));
    HitboxComponent3 = CreateHitboxComponent(TEXT("HitboxComponent3"), TEXT("hitbox3"));

    // Create enemy detection component
    EnemyDetectionComponent = CreateDefaultSubobject<UEnemyDetectionComponent>(
        TEXT("EnemyDetectionComponent"));
}

UCapsuleComponent* ABouncerBlockade::CreateHitboxComponent(const FName& ComponentName,
                                                           const FName& SocketName) {
    UCapsuleComponent* HitboxComponent = CreateDefaultSubobject<UCapsuleComponent>(ComponentName);
    HitboxComponent->SetupAttachment(SkeletonMesh, SocketName);
    SetupHitboxCollision(HitboxComponent);
    return HitboxComponent;
}

void ABouncerBlockade::SetupHitboxCollision(UCapsuleComponent* HitboxComponent) {
    UCollisionUtils::SetupAttackHitbox(HitboxComponent);
}

void ABouncerBlockade::OnHitboxOverlap(UPrimitiveComponent* OverlappedComponent,
                                       AActor* OtherActor,
                                       UPrimitiveComponent* OtherComp,
                                       int32 OtherBodyIndex,
                                       bool bFromSweep,
                                       const FHitResult& SweepResult) {
    // If the other actor is ourself, ignore it
    if (OtherActor == this) { return; }
    
    UDamageUtils::ApplyDamage(OtherActor, this, 20.0f, 1000.0f, EDDDamageType::Melee);
}

void ABouncerBlockade::OnHitboxBeginNotifyReceived(UAnimSequenceBase* animSequence) {
    SetHitboxCollisionEnabled(true);
}

void ABouncerBlockade::OnHitboxEndNotifyReceived(UAnimSequenceBase* animSequence) {
    SetHitboxCollisionEnabled(false);
}

// Const requirement disabled since it breaks compilation when using dynamic multicast delegates
// ReSharper disable once CppMemberFunctionMayBeConst
void ABouncerBlockade::OnAnimMontageEnded(UAnimMontage* Montage, bool bInterrupted) {
    // if the montage is the attack montage, set the animation state to idle
    if (AnimInstance->AttackMontage == Montage) {
        AnimInstance->CurrentBouncerBlockadeAnimationState = EBouncerBlockadeAnimationState::Idle;
    }
}

void ABouncerBlockade::BeginPlay() {
    Super::BeginPlay();
    
    // Set up overlap event bindings
    HitboxComponent1->OnComponentBeginOverlap.RemoveDynamic(
        this,
        &ABouncerBlockade::OnHitboxOverlap);
    HitboxComponent2->OnComponentBeginOverlap.RemoveDynamic(
        this,
        &ABouncerBlockade::OnHitboxOverlap);
    HitboxComponent3->OnComponentBeginOverlap.RemoveDynamic(
        this,
        &ABouncerBlockade::OnHitboxOverlap);
    HitboxComponent1->OnComponentBeginOverlap.AddDynamic(this, &ABouncerBlockade::OnHitboxOverlap);
    HitboxComponent2->OnComponentBeginOverlap.AddDynamic(this, &ABouncerBlockade::OnHitboxOverlap);
    HitboxComponent3->OnComponentBeginOverlap.AddDynamic(this, &ABouncerBlockade::OnHitboxOverlap);

    // Set up animation event bindings
    AnimInstance->OnMontageEnded.RemoveDynamic(this, &ABouncerBlockade::OnAnimMontageEnded);
    AnimInstance->OnMontageEnded.AddDynamic(this, &ABouncerBlockade::OnAnimMontageEnded);
}

void ABouncerBlockade::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);
    
    if (StructurePlacementState == EStructurePlacementState::Previewing) { return; }

    UpdateStructureState();
    UpdateAttackState();
}

void ABouncerBlockade::UpdateStructureState() {
    const FEnemyDetectionResult DetectionResult = EnemyDetectionComponent->DetectEnemies();
    CurrentState = DetectionResult.bEnemiesInRange
                       ? EDefensiveStructureState::Attacking
                       : EDefensiveStructureState::Idle;
}

void ABouncerBlockade::UpdateAttackState() {
    const bool bShouldAttack = CurrentState == EDefensiveStructureState::Attacking &&
                               AnimInstance->CurrentBouncerBlockadeAnimationState ==
                               EBouncerBlockadeAnimationState::Idle;

    if (bShouldAttack) { Attack(); }
}

void ABouncerBlockade::Attack() {
    AnimInstance->Montage_Play(AnimInstance->AttackMontage, 1.0f);
    AnimInstance->CurrentBouncerBlockadeAnimationState = EBouncerBlockadeAnimationState::Attacking;
}

void ABouncerBlockade::SetHitboxCollisionEnabled(bool bEnabled) const {
    const ECollisionEnabled::Type CollisionType = bEnabled
                                                      ? ECollisionEnabled::QueryAndPhysics
                                                      : ECollisionEnabled::NoCollision;
    HitboxComponent1->SetCollisionEnabled(CollisionType);
    HitboxComponent2->SetCollisionEnabled(CollisionType);
    HitboxComponent3->SetCollisionEnabled(CollisionType);
}

bool ABouncerBlockade::HasRequiredDependencies() const {
    if (!Super::HasRequiredDependencies()) { return false; }
    
    return AnimInstance != nullptr;
}

void ABouncerBlockade::CollectDependencies() {
    Super::CollectDependencies();
    
    if (!AnimInstance) {
        AnimInstance = Cast<UBouncerBlockadeAnimInstance>(BaseAnimInstance);
    }
}

void ABouncerBlockade::ValidateConfiguration() const {
    Super::ValidateConfiguration();
    
    ensureAlways(AnimInstance->AttackMontage != nullptr);
}

UEnemyDetectionComponent* ABouncerBlockade::GetEnemyDetectionComponent() {
    return EnemyDetectionComponent;
}
