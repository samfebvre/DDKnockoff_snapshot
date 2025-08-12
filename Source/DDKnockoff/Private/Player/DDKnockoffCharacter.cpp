#include "Player/DDKnockoffCharacter.h"

#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Player/PlayerInteractionComponent.h"
#include "Player/PlayerAnimInstance.h"
#include "Components/SphereComponent.h"
#include "Enemies/DDAICharacter.h"
#include "Collision/DDCollisionChannels.h"
#include "Core/DDKnockoffGameMode.h"
#include "Core/ManagerHandlerSubsystem.h"
#include "Debug/DebugInformationManager.h"
#include "Structures/DefensiveStructure.h"
#include "Structures/StructurePlacementManager.h"
#include "Structures/StructurePlacementSettings.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Currency/CurrencyCrystal.h"
#include "Currency/CurrencySpawner.h"
#include "Currency/CurrencyUtils.h"
#include "Entities/EntityData.h"
#include "Health/HealthComponent.h"
#include "Entities/EntityManager.h"
#include "Damage/DamageUtils.h"
#include "Utils/CollisionUtils.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ADDKnockoffCharacter


void ADDKnockoffCharacter::AddCurrency(int CurrencyAmount) {
    if (CurrencyAmount <= 0) { return; }

    // Calculate how much currency we can actually add
    const int AvailableSpace = MaxCurrency - CurrentCurrency;
    const int CurrencyToAdd = FMath::Min(CurrencyAmount, AvailableSpace);
    const int OverflowAmount = CurrencyAmount - CurrencyToAdd;

    // Add currency up to the limit
    CurrentCurrency += CurrencyToAdd;

    // Spawn overflow currency as crystals if there's excess
    if (OverflowAmount > 0) {
        UCurrencyUtils::SpawnCurrencyBurstFromActor(
            this,
            CurrencySpawner,
            OverflowAmount,
            MinimumOverflowCrystalCount,
            FVector::UpVector,
            // Spawn upward
            500.0f // Lower speed than enemy drops
            );
    }
}

void ADDKnockoffCharacter::SpendCurrency(int CurrencyAmount) {
    if (CurrentCurrency >= CurrencyAmount) { CurrentCurrency -= CurrencyAmount; } else {
        UE_LOG(LogTemplateCharacter, Warning, TEXT("Not enough currency to spend!"));
    }
}

bool ADDKnockoffCharacter::IsAtMaxCurrency() const { return CurrentCurrency >= MaxCurrency; }

void ADDKnockoffCharacter::TryCollectCurrencyCrystal(ACurrencyCrystal* CurrencyCrystal) {
    if (!CurrencyCrystal) { return; }

    // If player is at max currency, do nothing - don't collect or destroy crystal
    if (IsAtMaxCurrency()) { return; }

    // Player can collect currency, so add it and destroy the crystal
    AddCurrency(CurrencyCrystal->GetCurrencyAmount());
    CurrencyCrystal->Destroy();
}

ADDKnockoffCharacter::ADDKnockoffCharacter() {
    // Enable ticking for this character
    PrimaryActorTick.bCanEverTick = true;

    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
    GetCapsuleComponent()->SetCollisionResponseToChannel(DDCollisionChannels::ECC_Hurtbox,
                                                         ECR_Block);
    // TODO - Might want to ignore hitboxes actually and let the hurtbox handle it
    GetCapsuleComponent()->SetCollisionResponseToChannel(DDCollisionChannels::ECC_Hitbox,
                                                         ECR_Overlap);
    GetCapsuleComponent()->SetCollisionResponseToChannel(DDCollisionChannels::ECC_Collectibles,
                                                         ECR_Overlap);

    // Don't rotate when the controller rotates. Let that just affect the camera.
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // Configure character movement - handle rotation manually
    GetCharacterMovement()->bOrientRotationToMovement = false;

    // Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
    // instead of recompiling to adjust them
    GetCharacterMovement()->JumpZVelocity = 700.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 500.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

    // Create a camera boom (pulls in towards the player if there is a collision)
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    // The camera follows at this distance behind the character	
    CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

    // Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
    FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

    // Interaction component
    PlayerInteractionComponent = CreateDefaultSubobject<UPlayerInteractionComponent>(
        TEXT("PlayerInteractionComponent"));

    // Currency collector component
    CurrencyCollectorComponent = CreateDefaultSubobject<USphereComponent>(
        TEXT("CurrencyCollectorComponent"));
    CurrencyCollectorComponent->SetupAttachment(RootComponent);
    CurrencyCollectorComponent->SetSphereRadius(200.0f);
    CurrencyCollectorComponent->SetCollisionObjectType(DDCollisionChannels::ECC_Trigger);
    CurrencyCollectorComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    CurrencyCollectorComponent->SetCollisionResponseToChannel(
        DDCollisionChannels::ECC_Collectibles,
        ECR_Overlap);

    // Currency spawner for overflow
    CurrencySpawner = CreateDefaultSubobject<UCurrencySpawner>(TEXT("CurrencySpawner"));
    CurrencySpawner->SetupAttachment(RootComponent);

    // Attack hitbox component
    AttackHitboxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackHitbox"));
    AttackHitboxComponent->SetupAttachment(RootComponent);
    UCollisionUtils::SetupAttackHitbox(AttackHitboxComponent);

    // Sword mesh component
    SwordMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwordMesh"));
    SwordMeshComponent->SetupAttachment(GetMesh(), FName("hand_r")); // Attach to right hand socket
    SwordMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
    // are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

    // Create entity data
    EntityData = CreateDefaultSubobject<UEntityData>(TEXT("EntityData"));
}

//////////////////////////////////////////////////////////////////////////
// Input

void ADDKnockoffCharacter::Tick(const float DeltaTime) {
    Super::Tick(DeltaTime);

    switch (StructurePlacementState) {
        case EPreviewStructurePlacementState::None:
            break;
        case EPreviewStructurePlacementState::Previewing_Position: {
            StructurePlacementLocation = GetDesiredStructurePlacementLocation();

            // Move the structure preview to the desired location
            StructurePreview->SetActorLocation(StructurePlacementLocation);
            StructurePreview->SetActorRotation(StructurePlacementRotation);

            UpdatePlacementValidity();
            break;
        }
        case EPreviewStructurePlacementState::Previewing_Rotation: {
            StructurePreview->SetActorRotation(StructurePlacementRotation);
            UpdatePlacementValidity();
            break;
        }
        default: ;
    }

    ActorsOverlappingCurrencyCollector.Empty();
    CurrencyCollectorComponent->GetOverlappingActors(ActorsOverlappingCurrencyCollector);

    // For each currency crystal overlapping the collector, add a force to it to bring it towards us
    // Only apply magnetic force if player is not at max currency
    if (!IsAtMaxCurrency()) {
        for (AActor* Actor : ActorsOverlappingCurrencyCollector) {
            if (ACurrencyCrystal* CurrencyCrystal = Cast<ACurrencyCrystal>(Actor)) {
                CurrencyCrystal->UpdateAttraction(GetActorLocation());
            }
        }
    }

    // Apply root motion from curves during attacks
    if (AnimInstance && AnimInstance->CurrentPoseState == EPlayerPoseState::Attack) {
        ApplyRootMotionForAttack(DeltaTime);
    }

    // Handle character rotation manually
    UpdateCharacterRotation(DeltaTime);
}

void ADDKnockoffCharacter::LaunchTest() {
    // Launch character forward
    FVector LaunchDirection = GetActorForwardVector();
    LaunchDirection.Z = 1.0f; // Add some upward force
    LaunchDirection.Normalize();
    LaunchCharacter(LaunchDirection * 1000.f, true, true);
}

void ADDKnockoffCharacter::NotifyControllerChanged() {
    Super::NotifyControllerChanged();

    // Add Input Mapping Context
    if (const APlayerController* PlayerController = Cast<APlayerController>(Controller)) {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
            UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void ADDKnockoffCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
    // Set up action bindings
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(
        PlayerInputComponent)) {
        // Jumping
        EnhancedInputComponent->BindAction(JumpAction,
                                           ETriggerEvent::Started,
                                           this,
                                           &ACharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction,
                                           ETriggerEvent::Completed,
                                           this,
                                           &ACharacter::StopJumping);

        // Moving
        EnhancedInputComponent->BindAction(MoveAction,
                                           ETriggerEvent::Triggered,
                                           this,
                                           &ADDKnockoffCharacter::Move);

        // Looking
        EnhancedInputComponent->BindAction(LookAction,
                                           ETriggerEvent::Triggered,
                                           this,
                                           &ADDKnockoffCharacter::OnMouseMovement);

        // Structure one placement
        EnhancedInputComponent->BindAction(StartPlacingStructure1Action,
                                           ETriggerEvent::Triggered,
                                           this,
                                           &ADDKnockoffCharacter::StartPlacingStructure1);

        // Structure two placement
        EnhancedInputComponent->BindAction(StartPlacingStructure2Action,
                                           ETriggerEvent::Triggered,
                                           this,
                                           &ADDKnockoffCharacter::StartPlacingStructure2);

        // Structure three placement
        EnhancedInputComponent->BindAction(StartPlacingStructure3Action,
                                           ETriggerEvent::Triggered,
                                           this,
                                           &ADDKnockoffCharacter::StartPlacingStructure3);

        // Structure four placement
        EnhancedInputComponent->BindAction(StartPlacingStructure4Action,
                                           ETriggerEvent::Triggered,
                                           this,
                                           &ADDKnockoffCharacter::StartPlacingStructure4);

        // Structure five placement
        EnhancedInputComponent->BindAction(StartPlacingStructure5Action,
                                           ETriggerEvent::Triggered,
                                           this,
                                           &ADDKnockoffCharacter::StartPlacingStructure5);

        // Primary input
        EnhancedInputComponent->BindAction(PrimaryInputAction,
                                           ETriggerEvent::Triggered,
                                           this,
                                           &ADDKnockoffCharacter::OnPrimaryInputPressed);

        // Secondary input - should fire while the right mouse button is pressed
        EnhancedInputComponent->BindAction(SecondaryInputAction,
                                           ETriggerEvent::Triggered,
                                           this,
                                           &ADDKnockoffCharacter::OnSecondaryInput);

        // Mouse scroll
        EnhancedInputComponent->BindAction(RotatePreviewDuringPositioningPreviewInputAction,
                                           ETriggerEvent::Triggered,
                                           this,
                                           &ADDKnockoffCharacter::OnRotationInputDuringPreviewPositioning);

        // Ready Up input
        EnhancedInputComponent->BindAction(ReadyUpAction,
                                           ETriggerEvent::Triggered,
                                           this,
                                           &ADDKnockoffCharacter::OnReadyUpInputTriggered);

        // Attack input
        EnhancedInputComponent->BindAction(AttackAction,
                                           ETriggerEvent::Started,
                                           this,
                                           &ADDKnockoffCharacter::OnAttackInputPressed);

        // Cancel input
        EnhancedInputComponent->BindAction(CancelAction,
                                           ETriggerEvent::Started,
                                           this,
                                           &ADDKnockoffCharacter::OnCancelInputPressed);
    } else {
        UE_LOG(LogTemplateCharacter,
               Error,
               TEXT(
                   "'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."
               ),
               *GetNameSafe(this));
    }
}

bool ADDKnockoffCharacter::DeprojectScreenCentreToWorld(FVector& OutWorldLocation,
                                                        FVector& OutWorldDirection) const {
    // Get the viewport size (screen resolution)
    int32 ScreenWidth, ScreenHeight;
    CachedPlayerController->GetViewportSize(ScreenWidth, ScreenHeight);

    // Calculate the center of the screen
    const FVector2D ScreenCenter = FVector2D(ScreenWidth / 2.0f, ScreenHeight / 2.0f);

    // Deproject the screen center into world space
    const auto ret = CachedPlayerController->DeprojectScreenPositionToWorld(
        ScreenCenter.X,
        ScreenCenter.Y,
        OutWorldLocation,
        OutWorldDirection);

    return ret;
}

FVector ADDKnockoffCharacter::FlattenZ(const FVector& Vector) {
    return FVector(Vector.X, Vector.Y, 0.0f);
}


FVector ADDKnockoffCharacter::GetCapsuleBottomLocation() const {
    // Get the location of the bottom of the capsule component
    return GetActorLocation() - FVector(0.f,
                                        0.f,
                                        GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
}

FVector ADDKnockoffCharacter::GetDefaultFallbackPlacementLocation() const {
    return GetActorLocation() + GetActorForwardVector() * 200.f; // Default fallback location 
}

void ADDKnockoffCharacter::OnCapsuleOverlapBegin(UPrimitiveComponent* OverlappedComponent,
                                                 AActor* OtherActor,
                                                 UPrimitiveComponent* OtherComp,
                                                 int32 OtherBodyIndex,
                                                 bool bFromSweep,
                                                 const FHitResult& SweepResult) {
    // Check if the overlapping actor is a currency crystal
    if (OtherActor && OtherActor->IsA(ACurrencyCrystal::StaticClass())) {
        // Cast to the currency crystal and attempt collection
        if (ACurrencyCrystal* CurrencyCrystal = Cast<ACurrencyCrystal>(OtherActor)) {
            TryCollectCurrencyCrystal(CurrencyCrystal);
        }
    }
}

void ADDKnockoffCharacter::BeginPlay() {
    Super::BeginPlay();
    CachedPlayerController = Cast<APlayerController>(GetController());

    // Handle dependency injection with single call
    EnsureDependenciesInjected();

    // Validate editor-configured properties
    ValidateConfiguration();

    // Register with EntityManager using injected dependency
    EntityManager->RegisterEntity(this);

    // Initialize and validate animation instance
    UAnimInstance* BaseAnimInstance = GetMesh()->GetAnimInstance();
    ensureAlways(BaseAnimInstance != nullptr);

    AnimInstance = Cast<UPlayerAnimInstance>(BaseAnimInstance);
    ensureAlways(
        AnimInstance != nullptr && "Animation instance must be of type UPlayerAnimInstance");

    GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(
        this,
        &ADDKnockoffCharacter::OnCapsuleOverlapBegin);

    // Bind attack hitbox overlap event
    AttackHitboxComponent->OnComponentBeginOverlap.AddDynamic(
        this,
        &ADDKnockoffCharacter::OnAttackHitboxOverlap);

    // Register with DebugManager using injected dependency
    DebugManager->RegisterDebugInformationProvider(this);
}

void ADDKnockoffCharacter::EndPlay(EEndPlayReason::Type EndPlayReason) {
    // Unregister from EntityManager using injected dependency
    if (EntityManager) { EntityManager->UnregisterEntity(this); }

    // Unregister from DebugManager using injected dependency
    if (DebugManager) { DebugManager->UnregisterDebugInformationProvider(this); }

    Super::EndPlay(EndPlayReason);
}

void ADDKnockoffCharacter::SetStructurePlacementState(EPreviewStructurePlacementState NewState) {
    StructurePlacementState = NewState;
}

FString ADDKnockoffCharacter::GetDebugCategory() const { return TEXT("Character"); }

FString ADDKnockoffCharacter::GetDebugInformation() const {
    // return currency and, if previewing a structure, the placement state and validity with invalidity reason
    FString DebugInfo = FString::Printf(TEXT("Currency: %d\n"), CurrentCurrency);
    if (StructurePlacementState != EPreviewStructurePlacementState::None) {
        DebugInfo += FString::Printf(TEXT("Structure Placement State: %s\n"),
                                     *UEnum::GetValueAsString(StructurePlacementState).Replace(
                                         TEXT("EPreviewStructurePlacementState::"),
                                         TEXT("")));
        DebugInfo += FString::Printf(TEXT("Validity State: %s\n"),
                                     *UEnum::GetValueAsString(StructurePlacementValidityState).
                                     Replace(TEXT("EStructurePlacementValidityState::"), TEXT("")));
        DebugInfo += FString::Printf(TEXT("Invalidity Reason: %s\n"),
                                     *UEnum::GetValueAsString(StructurePlacementInvalidityReason).
                                     Replace(TEXT("EStructurePlacementInvalidityReason::"),
                                             TEXT("")));
    }
    return DebugInfo;
}

void ADDKnockoffCharacter::Move(const FInputActionValue& Value) {
    // input is a Vector2D
    const FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr) {
        // find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // get forward vector
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

        // get right vector 
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // add movement 
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);

        // Track input direction for attack rotation (separate from velocity)
        LastMovementInput = FVector(MovementVector.X, MovementVector.Y, 0.0f);
    }
}

void ADDKnockoffCharacter::OnMouseMovement(const FInputActionValue& Value) {
    // input is a Vector2D
    const FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (StructurePlacementState == EPreviewStructurePlacementState::Previewing_Rotation) {
        // Rotate the structure preview based on mouse movement
        const UStructurePlacementSettings& Settings = PlacementSubsystem->GetPlacementSettings();
        const float RotationSpeed = Settings.RotationPhaseRotationSpeed;
        StructurePlacementRotation.Yaw += LookAxisVector.X * RotationSpeed;
    }
    // Look around
    else if (Controller != nullptr) {
        // add yaw and pitch input to controller
        AddControllerYawInput(LookAxisVector.X);
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void ADDKnockoffCharacter::OnPrimaryInputPressed(const FInputActionValue& Value) {
    switch (StructurePlacementState) {
        case EPreviewStructurePlacementState::None:
            break;

        case EPreviewStructurePlacementState::Previewing_Position:
        case EPreviewStructurePlacementState::Previewing_Rotation: {
            if (StructurePlacementValidityState == EStructurePlacementValidityState::Invalid) {
                // Invalid placement, do not place the structure
                return;
            }

            // Confirm rotation, place the structure
            SetStructurePlacementState(EPreviewStructurePlacementState::None);

            StructurePreview->OnPlaced();

            CurrentCurrency -= StructurePreview->GetCurrencyCostToPlaceStructure();

            StructurePreview = nullptr;
            break;
        }
        default: ;
    }
}

void ADDKnockoffCharacter::OnSecondaryInput(const FInputActionValue& Value) {
    switch (StructurePlacementState) {
        case EPreviewStructurePlacementState::None:
            break;
        case EPreviewStructurePlacementState::Previewing_Position:
            if (Value.Get<bool>() == true) {
                SetStructurePlacementState(EPreviewStructurePlacementState::Previewing_Rotation);
            }
            break;
        case EPreviewStructurePlacementState::Previewing_Rotation:
            if (Value.Get<bool>() == false) {
                SetStructurePlacementState(EPreviewStructurePlacementState::Previewing_Position);
            }
            break;
        default: ;
    }
}

void ADDKnockoffCharacter::
OnRotationInputDuringPreviewPositioning(const FInputActionValue& Value) {
    // input is a float
    const float MovementFloat = Value.Get<float>();
    const UStructurePlacementSettings& Settings = PlacementSubsystem->GetPlacementSettings();
    const float RotationSpeed = Settings.PositioningPhaseRotationSpeed;
    StructurePlacementRotation.Yaw += MovementFloat * RotationSpeed;
}

void ADDKnockoffCharacter::UpdatePlacementValidity() {
    StructurePlacementInvalidityReason = EStructurePlacementInvalidityReason::None;

    if (StructurePreview->GetCurrencyCostToPlaceStructure() > CurrentCurrency) {
        StructurePlacementInvalidityReason |= EStructurePlacementInvalidityReason::CannotAfford;
    }

    ActorsOverlappingPreview.Empty();
    StructurePreview->GetPreviewCollisionMesh()->GetOverlappingActors(ActorsOverlappingPreview);

    for (const auto& Actor : ActorsOverlappingPreview) {
        if (Actor != StructurePreview) {
            StructurePlacementInvalidityReason |= EStructurePlacementInvalidityReason::Overlapping;
            break;
        }
    }

    if (!PreviewRaycastHitResult.IsValidBlockingHit()) {
        StructurePlacementInvalidityReason |= EStructurePlacementInvalidityReason::NoGroundFound;
    } else {
        // Check surface normal for flatness
        const auto NormalDot = FVector::DotProduct(PreviewRaycastHitResult.ImpactNormal,
                                                   FVector(0.f, 0.f, 1.f));
        const auto Angle = FMath::Acos(NormalDot) * (180.f / PI);

        // Get characters walkable floor angle
        const auto WalkableFloorAngle = GetCharacterMovement()->GetWalkableFloorAngle();
        if (Angle > WalkableFloorAngle) {
            StructurePlacementInvalidityReason |= EStructurePlacementInvalidityReason::Angle;
        }

        // Check hit distance is within limits
        const auto DistanceToHit = PreviewRaycastHitResult.Distance;
        // Check if the distance is within the max distance from the camera
        const UStructurePlacementSettings& Settings = PlacementSubsystem->GetPlacementSettings();
        const float MaxDistance = Settings.MaxPlacementDistance;
        if (DistanceToHit > MaxDistance) {
            StructurePlacementInvalidityReason |= EStructurePlacementInvalidityReason::Distance;
        }
    }

    StructurePlacementValidityState = StructurePlacementInvalidityReason ==
                                      EStructurePlacementInvalidityReason::None
                                          ? EStructurePlacementValidityState::Valid
                                          : EStructurePlacementValidityState::Invalid;

    UpdatePreviewColor();
}

void ADDKnockoffCharacter::UpdatePreviewColor() const {
    if (StructurePreview) {
        StructurePreview->UpdatePreviewMaterialColor(StructurePlacementValidityState,
                                                     StructurePlacementInvalidityReason);
    }
}

void ADDKnockoffCharacter::StartPlacingStructure(
    const TSubclassOf<ADefensiveStructure>& StructureClass) {
    if (StructurePreview != NULL) { StructurePreview->Destroy(); }

    // Spawn the structure in front of the character
    SetStructurePlacementState(EPreviewStructurePlacementState::Previewing_Position);
    UE_LOG(LogTemplateCharacter,
           Log,
           TEXT("Start placing structure, entering Previewing_Position state."));

    // Get the player controller and character's position
    const FVector SpawnLocation = GetDesiredStructurePlacementLocation();
    StructurePlacementLocation = SpawnLocation;

    // Camera forward vec
    const auto CameraForward = FollowCamera->GetComponentRotation().Vector();
    const auto FlattenedCameraForward = FlattenZ(CameraForward);
    const FRotator SpawnRotation = FlattenedCameraForward.Rotation();
    StructurePlacementRotation = SpawnRotation;

    // Spawn the defensive structure
    const FActorSpawnParameters SpawnParams;
    StructurePreview = GetWorld()->SpawnActor<ADefensiveStructure>(
        StructureClass,
        StructurePlacementLocation,
        StructurePlacementRotation,
        SpawnParams);

    StructurePreview->OnStartedPreviewing();

    UpdatePlacementValidity();
}

void ADDKnockoffCharacter::CancelStructurePlacement() {
    // Only cancel if we're actually in placement mode
    if (StructurePlacementState == EPreviewStructurePlacementState::None) { return; }

    // Clean up the preview actor
    if (StructurePreview) {
        StructurePreview->Destroy();
        StructurePreview = nullptr;
    }

    // Reset placement state
    SetStructurePlacementState(EPreviewStructurePlacementState::None);

    // Clear placement data
    StructurePlacementLocation = FVector::ZeroVector;
    StructurePlacementRotation = FRotator::ZeroRotator;
    ActorsOverlappingPreview.Empty();
    StructurePlacementValidityState = EStructurePlacementValidityState::None;
    StructurePlacementInvalidityReason = EStructurePlacementInvalidityReason::None;

    UE_LOG(LogTemplateCharacter, Log, TEXT("Structure placement cancelled"));
}

void ADDKnockoffCharacter::StartPlacingStructure1(const FInputActionValue& Value) {
    StartPlacingStructure(DefensiveStructureClass1);
}

void ADDKnockoffCharacter::StartPlacingStructure2(const FInputActionValue& Value) {
    StartPlacingStructure(DefensiveStructureClass2);
}

void ADDKnockoffCharacter::StartPlacingStructure3(const FInputActionValue& Value) {
    StartPlacingStructure(DefensiveStructureClass3);
}

void ADDKnockoffCharacter::StartPlacingStructure4(const FInputActionValue& Value) {
    StartPlacingStructure(DefensiveStructureClass4);
}

void ADDKnockoffCharacter::StartPlacingStructure5(const FInputActionValue& Value) {
    StartPlacingStructure(DefensiveStructureClass5);
}

FVector ADDKnockoffCharacter::GetDesiredStructurePlacementLocation() {
    FVector Start = FollowCamera->GetComponentLocation();
    FVector Direction = FollowCamera->GetForwardVector();

    if (DeprojectScreenCentreToWorld(Start, Direction)) {
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        Params.AddIgnoredActor(StructurePreview);

        constexpr float MaxRaycastDistance = 10000.0f;
        const FVector End = Start + Direction * MaxRaycastDistance;

        if (FHitResult Result; GetWorld()->LineTraceSingleByChannel(Result,
            Start,
            End,
            ECC_Visibility,
            Params)) {
            PreviewRaycastHitResult = Result;
            const FVector HitLocation = PreviewRaycastHitResult.ImpactPoint;

            DrawDebugSphere(GetWorld(), HitLocation, 10.0f, 12, FColor::Green, false);
            DrawDebugLine(GetWorld(),
                          HitLocation,
                          HitLocation + PreviewRaycastHitResult.ImpactNormal * 100,
                          FColor::Green,
                          false,
                          0,
                          0,
                          5.f);
            return HitLocation;
        }
    }

    return GetDefaultFallbackPlacementLocation();
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ADDKnockoffCharacter::OnReadyUpInputPressed() {
    if (ADDKnockoffGameMode* GameMode = Cast<ADDKnockoffGameMode>(GetWorld()->GetAuthGameMode())) {
        GameMode->OnCharacterPressedReadyUpInput();
    }
}

void ADDKnockoffCharacter::OnReadyUpInputTriggered(const FInputActionValue& Value) {
    if (Value.Get<bool>()) { OnReadyUpInputPressed(); } else { OnReadyUpInputReleased(); }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ADDKnockoffCharacter::OnReadyUpInputReleased() {
    if (ADDKnockoffGameMode* GameMode = Cast<ADDKnockoffGameMode>(GetWorld()->GetAuthGameMode())) {
        GameMode->OnCharacterReleasedReadyUpInput();
    }
}

// IDependencyInjectable interface implementation
bool ADDKnockoffCharacter::HasRequiredDependencies() const {
    return PlacementSubsystem != nullptr && EntityManager != nullptr && DebugManager != nullptr;
}

void ADDKnockoffCharacter::CollectDependencies() {
    // Only collect PlacementSubsystem if not already set (e.g., from manual injection)
    if (!PlacementSubsystem) {
        // Get PlacementSubsystem from game instance subsystem (normal gameplay fallback)
        PlacementSubsystem = UManagerHandlerSubsystem::GetManager<UStructurePlacementManager>(
            GetWorld());
    }

    // Only collect EntityManager if not already set (e.g., from manual injection)
    if (!EntityManager) {
        // Get EntityManager from world subsystem (normal gameplay fallback)
        EntityManager = UManagerHandlerSubsystem::GetManager<UEntityManager>(GetWorld());
    }

    // Only collect DebugManager if not already set (e.g., from manual injection)
    if (!DebugManager) {
        // Get DebugManager from world subsystem (normal gameplay fallback)
        DebugManager = UManagerHandlerSubsystem::GetManager<UDebugInformationManager>(GetWorld());
    }
}

// IConfigurationValidatable interface implementation
void ADDKnockoffCharacter::ValidateConfiguration() const {
    // Validate Input Actions
    ensureAlways(DefaultMappingContext != nullptr);
    ensureAlways(JumpAction != nullptr);
    ensureAlways(MoveAction != nullptr);
    ensureAlways(LookAction != nullptr);
    ensureAlways(StartPlacingStructure1Action != nullptr);
    ensureAlways(StartPlacingStructure2Action != nullptr);
    ensureAlways(StartPlacingStructure3Action != nullptr);
    ensureAlways(StartPlacingStructure4Action != nullptr);
    ensureAlways(StartPlacingStructure5Action != nullptr);
    ensureAlways(PrimaryInputAction != nullptr);
    ensureAlways(SecondaryInputAction != nullptr);
    ensureAlways(RotatePreviewDuringPositioningPreviewInputAction != nullptr);
    ensureAlways(ReadyUpAction != nullptr);
    ensureAlways(AttackAction != nullptr);
    ensureAlways(CancelAction != nullptr);

    // Validate combat properties
    ensureAlways(MeleeDamage > 0.0f);
    ensureAlways(KnockbackStrength > 0.0f);
    ensureAlways(ComboInputWindow > 0.0f);
    ensureAlways(ComboInputWindow <= 2.0f); // Reasonable upper limit

    // Validate Defense Structure Classes
    ensureAlways(DefensiveStructureClass1 != nullptr);
    ensureAlways(DefensiveStructureClass2 != nullptr);
    ensureAlways(DefensiveStructureClass3 != nullptr);
    ensureAlways(DefensiveStructureClass4 != nullptr);
    ensureAlways(DefensiveStructureClass5 != nullptr);
}

void ADDKnockoffCharacter::Attack() {
    PrepareRootMotionBlendVariables();
    StartAttackRotation();
    AnimInstance->Attack();
}

bool ADDKnockoffCharacter::CanAttack() const { return AnimInstance && AnimInstance->CanAttack(); }

void ADDKnockoffCharacter::EnterHitReaction() const {
    AnimInstance->EnterHitReaction();
    GetCharacterMovement()->StopMovementImmediately();
}

// Attack input handler
void ADDKnockoffCharacter::OnAttackInputPressed(const FInputActionValue& Value) {
    // Check if we can attack and trigger attack
    if (CanAttack()) {
        bShouldBlendRootMotionForAttack =
            AnimInstance->CurrentPoseState == EPlayerPoseState::Locomotion;
        Attack();
    } else if (AnimInstance && AnimInstance->CurrentPoseState == EPlayerPoseState::Attack &&
               AnimInstance->CurrentAttackState == EPlayerAttackState::Attacking_PreComboTrigger) {
        // Buffer input during attack for combo
        bComboInputBuffered = true;
    }
}


void ADDKnockoffCharacter::OnComboTrigger() {
    // Execute buffered combo if input was buffered
    if (bComboInputBuffered) {
        bComboInputBuffered = false;
        bShouldBlendRootMotionForAttack = false;
        Attack();
        // GetWorld()->GetTimerManager().ClearTimer(ComboWindowTimerHandle);
    } else { AnimInstance->SetAttackState(EPlayerAttackState::Attacking_PostComboTrigger); }
}

void ADDKnockoffCharacter::OnComboWindowBegin() const {
    AnimInstance->SetAttackState(EPlayerAttackState::Attacking_PreComboTrigger);
}

void ADDKnockoffCharacter::OnCancelInputPressed(const FInputActionValue& Value) {
    // Cancel structure placement if we're currently in placement mode
    if (StructurePlacementState != EPreviewStructurePlacementState::None) {
        CancelStructurePlacement();
    }
}

// Hitbox overlap handler
void ADDKnockoffCharacter::OnAttackHitboxOverlap(UPrimitiveComponent* OverlappedComponent,
                                                 AActor* OtherActor,
                                                 UPrimitiveComponent* OtherComp,
                                                 int32 OtherBodyIndex,
                                                 bool bFromSweep,
                                                 const FHitResult& SweepResult) {
    // Apply melee damage with knockback using centralized utility
    UDamageUtils::ApplyDamage(OtherActor,
                              this,
                              MeleeDamage,
                              KnockbackStrength,
                              EDDDamageType::Melee);
}

// IEntity interface implementation
void ADDKnockoffCharacter::TakeDamage(const FDamagePayload& DamagePayload) {
    // Find health component and apply damage
    if (UHealthComponent* HealthComp = FindComponentByClass<UHealthComponent>()) {
        HealthComp->TakeDamage(DamagePayload.DamageAmount);
    }

    UE_LOG(LogTemplateCharacter,
           Log,
           TEXT("Player took %f damage of type %s"),
           DamagePayload.DamageAmount,
           *UEnum::GetValueAsString(DamagePayload.DamageType));
}

void ADDKnockoffCharacter::OnHitboxBeginNotifyReceived(UAnimSequenceBase* AnimSequence) {
    // Enable the attack hitbox
    if (AttackHitboxComponent) {
        AttackHitboxComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}

void ADDKnockoffCharacter::OnHitboxEndNotifyReceived(UAnimSequenceBase* AnimSequence) {
    // Disable the attack hitbox
    if (AttackHitboxComponent) {
        AttackHitboxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void ADDKnockoffCharacter::TakeKnockback(const FVector& Direction, const float Strength) {
    LaunchCharacter(Direction * Strength, true, true);
    EnterHitReaction();
}

void ADDKnockoffCharacter::ApplyRootMotionForAttack(float DeltaTime) const {
    if (!AnimInstance) { return; }

    // Get root motion velocity from curves
    if (const FVector RootMotionVelocity = GetRootMotionVelocityFromCurves(); !RootMotionVelocity.
        IsZero()) {
        // Transform velocity from local to world space
        FVector WorldRootMotionVelocity = GetActorTransform().TransformVectorNoScale(
            RootMotionVelocity);

        WorldRootMotionVelocity.Z = GetCharacterMovement()->Velocity.Z;
        // Preserve Y velocity so that gravity still takes effect

        if (!bShouldBlendRootMotionForAttack) {
            GetCharacterMovement()->Velocity = WorldRootMotionVelocity;
            return;
        }

        const float ElapsedTime = GetWorld()->GetTimeSeconds() - AttackStartTime;
        float BlendAlpha = 1.0f;

        if (RootMotionBlendInDuration > 0.0f) {
            BlendAlpha = FMath::Clamp(ElapsedTime / RootMotionBlendInDuration, 0.0f, 1.0f);

            // Apply curve if available
            if (RootMotionBlendCurve) {
                BlendAlpha = RootMotionBlendCurve->GetFloatValue(BlendAlpha);
            }
        }

        // Single interpolation: blend from initial velocity directly to target
        const FVector TargetVelocity =
            FMath::Lerp(InitialAttackVelocity, WorldRootMotionVelocity, BlendAlpha);

        // Apply the target velocity directly
        GetCharacterMovement()->Velocity = TargetVelocity;
    }
}

FVector ADDKnockoffCharacter::GetRootMotionVelocityFromCurves() const {
    if (!AnimInstance) { return FVector::ZeroVector; }

    // Get velocity components from animation curves
    const float VelocityX = AnimInstance->GetCurveValue("RootMotion_VelocityX");
    const float VelocityY = AnimInstance->GetCurveValue("RootMotion_VelocityY");
    const float VelocityZ = AnimInstance->GetCurveValue("RootMotion_VelocityZ");

    // Handle coordinate system transformation
    // Swap X and Y for coordinate system conversion
    const FVector Result = FVector(VelocityY, VelocityX, VelocityZ);

    return Result;
}

float ADDKnockoffCharacter::GetRootMotionSpeedFromCurve() const {
    if (!AnimInstance) { return 0.0f; }

    return AnimInstance->GetCurveValue("RootMotion_Speed");
}

void ADDKnockoffCharacter::PrepareRootMotionBlendVariables() {
    // Store initial velocity for blending
    InitialAttackVelocity = GetCharacterMovement()->Velocity;
    AttackStartTime = GetWorld()->GetTimeSeconds();
}

void ADDKnockoffCharacter::UpdateCharacterRotation(float DeltaTime) {
    // Handle attack rotation burst (overrides normal rotation)
    if (bIsPerformingAttackRotation) {
        const float ElapsedTime = GetWorld()->GetTimeSeconds() - AttackStartTime;

        // Check if attack rotation burst is finished
        if (ElapsedTime >= AttackRotationDuration) {
            bIsPerformingAttackRotation = false;
            return;
        }

        // Calculate rotation speed falloff (starts at max, fades to zero)
        float RotationAlpha = ElapsedTime / AttackRotationDuration;
        if (AttackRotationCurve) {
            RotationAlpha = AttackRotationCurve->GetFloatValue(RotationAlpha);
        }

        // Rotation speed starts high and fades to zero
        const float CurrentRotationSpeed = AttackRotationSpeed * (1.0f - RotationAlpha);

        // Rotate toward the attack target direction
        const FRotator NewRotation = FMath::RInterpTo(
            GetActorRotation(),
            AttackTargetRotation,
            DeltaTime,
            CurrentRotationSpeed
            );

        SetActorRotation(NewRotation);
        return;
    }

    // Normal locomotion rotation (when not attacking)
    if (AnimInstance && AnimInstance->CurrentPoseState == EPlayerPoseState::Attack) {
        // During attacks (but not during attack rotation burst), don't rotate
        return;
    }

    // Only rotate if we're moving above the threshold
    const FVector CurrentVelocity = GetVelocity();
    if (CurrentVelocity.Size2D() < MovementThreshold) { return; }

    // Calculate target rotation from movement direction
    const FVector MovementDirection = CurrentVelocity.GetSafeNormal2D();
    const FRotator TargetRotation = MovementDirection.Rotation();

    // Smoothly rotate toward movement direction at normal speed
    const FRotator NewRotation = FMath::RInterpTo(
        GetActorRotation(),
        TargetRotation,
        DeltaTime,
        NormalRotationSpeed
        );

    SetActorRotation(NewRotation);
}

void ADDKnockoffCharacter::StartAttackRotation() {
    // Only start attack rotation if player has input direction
    if (LastMovementInput.IsNearlyZero()) {
        bIsPerformingAttackRotation = false;
        return;
    }

    // Convert input direction to world space direction
    const FRotator ControllerRotation = GetControlRotation();
    const FRotator YawRotation(0, ControllerRotation.Yaw, 0);

    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    // Calculate desired world direction from input
    const FVector DesiredDirection = (ForwardDirection * LastMovementInput.Y + RightDirection *
                                      LastMovementInput.X).GetSafeNormal();

    if (!DesiredDirection.IsNearlyZero()) {
        // Set target rotation and start attack rotation burst
        AttackTargetRotation = DesiredDirection.Rotation();
        bIsPerformingAttackRotation = true;
        // AttackStartTime is already set by PrepareRootMotionBlendVariables()
    }
}

EFaction ADDKnockoffCharacter::GetFaction() const { return EFaction::Player; }

AActor* ADDKnockoffCharacter::GetActor() { return this; }
EEntityType ADDKnockoffCharacter::GetEntityType() const { return EEntityType::Character; }
UEntityData* ADDKnockoffCharacter::GetEntityData() const { return EntityData; }
