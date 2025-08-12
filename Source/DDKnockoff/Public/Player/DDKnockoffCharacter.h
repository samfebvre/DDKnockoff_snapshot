#pragma once

#include "CoreMinimal.h"
#include "Debug/DebugInformationProvider.h"
#include "Structures/StructurePlacementEnums.h"
#include "Core/DependencyInjectable.h"
#include "Core/ConfigurationValidatable.h"
#include "Damage/DamagePayload.h"
#include "Entities/Entity.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "DDKnockoffCharacter.generated.h"

class USphereComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UPlayerInteractionComponent;
class UPlayerAnimInstance;
class UCurrencySpawner;
class UBoxComponent;
class ADefensiveStructure;
class UStructurePlacementManager;
class UEntityManager;
class UDebugInformationManager;
class UEntityData;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 * Main player character class with combat, structure placement, and currency collection capabilities.
 * Implements dependency injection for manager services and provides debug information.
 */
UCLASS(config=Game)
class DDKNOCKOFF_API ADDKnockoffCharacter
    : public ACharacter, public IDebugInformationProvider, public IDependencyInjectable,
      public IConfigurationValidatable, public IEntity {
    GENERATED_BODY()

public:
    ADDKnockoffCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION()
    void OnCapsuleOverlapBegin(UPrimitiveComponent* OverlappedComponent,
                               AActor* OtherActor,
                               UPrimitiveComponent* OtherComp,
                               int32 OtherBodyIndex,
                               bool bFromSweep,
                               const FHitResult& SweepResult);

    /**
     * Add currency to the player's inventory, spawning overflow as crystals if at capacity.
     * @param CurrencyAmount - Amount of currency to add
     */
    void AddCurrency(int CurrencyAmount);

    /**
     * Spend currency if available.
     * @param CurrencyAmount - Amount of currency to spend
     */
    void SpendCurrency(int CurrencyAmount);

    /**
     * Check if the player is at maximum currency capacity.
     * @return True if current currency equals max currency
     */
    UFUNCTION(BlueprintPure, Category = "Currency")
    bool IsAtMaxCurrency() const;

private:
    /**
     * Handle collection of a currency crystal, checking if player can collect it.
     * If at max currency, does nothing. Otherwise, adds currency and destroys crystal.
     * @param CurrencyCrystal - The crystal to attempt to collect
     */
    void TryCollectCurrencyCrystal(class ACurrencyCrystal* CurrencyCrystal);

public:
    void LaunchTest();
    void SetStructurePlacementState(EPreviewStructurePlacementState NewState);

    // Combo system
    UFUNCTION()
    void OnComboTrigger();

    UFUNCTION()
    void OnComboWindowBegin() const;

    // Root motion curve utilities
    void ApplyRootMotionForAttack(float DeltaTime) const;
    FVector GetRootMotionVelocityFromCurves() const;
    float GetRootMotionSpeedFromCurve() const;
    void PrepareRootMotionBlendVariables();

    // Character rotation utilities
    void UpdateCharacterRotation(float DeltaTime);
    void StartAttackRotation();

    // Camera accessors
    FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

    // IDebugInformationProvider Interface Implementation
    virtual FString GetDebugCategory() const override;
    virtual FString GetDebugInformation() const override;

    // IDependencyInjectable Interface Implementation
    virtual bool HasRequiredDependencies() const override;
    virtual void CollectDependencies() override;

    // IConfigurationValidatable Interface Implementation
    virtual void ValidateConfiguration() const override;

    // IEntity Interface Implementation
    virtual void TakeDamage(const FDamagePayload& DamagePayload) override;
    virtual void OnHitboxBeginNotifyReceived(UAnimSequenceBase* AnimSequence) override;
    virtual void OnHitboxEndNotifyReceived(UAnimSequenceBase* AnimSequence) override;
    virtual void TakeKnockback(const FVector& Direction, const float Strength) override;
    virtual EFaction GetFaction() const override;
    virtual AActor* GetActor() override;
    virtual EEntityType GetEntityType() const override;
    virtual UEntityData* GetEntityData() const override;

protected:
    virtual void NotifyControllerChanged() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    // Input handling
    void Move(const FInputActionValue& Value);
    void OnMouseMovement(const FInputActionValue& Value);
    void OnPrimaryInputPressed(const FInputActionValue& Value);
    void OnSecondaryInput(const FInputActionValue& Value);
    void OnRotationInputDuringPreviewPositioning(const FInputActionValue& Value);
    void OnReadyUpInputTriggered(const FInputActionValue& Value);
    void OnReadyUpInputPressed();
    void OnReadyUpInputReleased();
    void OnAttackInputPressed(const FInputActionValue& Value);
    void OnCancelInputPressed(const FInputActionValue& Value);

    // Structure placement
    void StartPlacingStructure1(const FInputActionValue& Value);
    void StartPlacingStructure2(const FInputActionValue& Value);
    void StartPlacingStructure3(const FInputActionValue& Value);
    void StartPlacingStructure4(const FInputActionValue& Value);
    void StartPlacingStructure5(const FInputActionValue& Value);
    void StartPlacingStructure(const TSubclassOf<ADefensiveStructure>& StructureClass);
    void CancelStructurePlacement();
    void UpdatePlacementValidity();
    void UpdatePreviewColor() const;
    FVector GetDesiredStructurePlacementLocation();
    FVector GetDefaultFallbackPlacementLocation() const;

    // Combat system
    void Attack();
    bool CanAttack() const;
    void EnterHitReaction() const;

    UFUNCTION()
    void OnAttackHitboxOverlap(UPrimitiveComponent* OverlappedComponent,
                               AActor* OtherActor,
                               UPrimitiveComponent* OtherComp,
                               int32 OtherBodyIndex,
                               bool bFromSweep,
                               const FHitResult& SweepResult);

    // Utility functions
    bool DeprojectScreenCentreToWorld(FVector& OutWorldLocation, FVector& OutWorldDirection) const;

    UFUNCTION(BlueprintCallable, Category = "Character")
    static FVector FlattenZ(const FVector& Vector);

    UFUNCTION(BlueprintCallable, Category = "Character")
    FVector GetCapsuleBottomLocation() const;

    // Components
    UPROPERTY(VisibleAnywhere,
        BlueprintReadOnly,
        Category = Camera,
        meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere,
        BlueprintReadOnly,
        Category = Camera,
        meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FollowCamera;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Interaction",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UPlayerInteractionComponent> PlayerInteractionComponent;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Currency",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USphereComponent> CurrencyCollectorComponent;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Currency",
        meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCurrencySpawner> CurrencySpawner;

    UPROPERTY(VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Combat",
        meta = (AllowPrivateAccess = "true"))
    UBoxComponent* AttackHitboxComponent;

    UPROPERTY(VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Combat",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* SwordMeshComponent;


    UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UPlayerAnimInstance> AnimInstance;

    UPROPERTY(Transient, Instanced)
    TObjectPtr<UEntityData> EntityData;

    // Input configuration
    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* JumpAction;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* MoveAction;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* LookAction;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* StartPlacingStructure1Action;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* StartPlacingStructure2Action;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* StartPlacingStructure3Action;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* StartPlacingStructure4Action;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* StartPlacingStructure5Action;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* PrimaryInputAction;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* SecondaryInputAction;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* RotatePreviewDuringPositioningPreviewInputAction;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* ReadyUpAction;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* AttackAction;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = Input,
        meta = (AllowPrivateAccess = "true"))
    UInputAction* CancelAction;

    // Structure classes
    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Structures",
        meta = (AllowPrivateAccess = "true"))
    TSubclassOf<ADefensiveStructure> DefensiveStructureClass1;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Structures",
        meta = (AllowPrivateAccess = "true"))
    TSubclassOf<ADefensiveStructure> DefensiveStructureClass2;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Structures",
        meta = (AllowPrivateAccess = "true"))
    TSubclassOf<ADefensiveStructure> DefensiveStructureClass3;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Structures",
        meta = (AllowPrivateAccess = "true"))
    TSubclassOf<ADefensiveStructure> DefensiveStructureClass4;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Structures",
        meta = (AllowPrivateAccess = "true"))
    TSubclassOf<ADefensiveStructure> DefensiveStructureClass5;

    // Combat configuration
    UPROPERTY(EditAnywhere,
        BlueprintReadWrite,
        Category = "Combat",
        meta = (AllowPrivateAccess = "true"))
    float MeleeDamage = 50.0f;

    UPROPERTY(EditAnywhere,
        BlueprintReadWrite,
        Category = "Combat",
        meta = (AllowPrivateAccess = "true"))
    float KnockbackStrength = 300.0f;

    UPROPERTY(EditAnywhere,
        BlueprintReadOnly,
        Category = "Combat",
        meta = (AllowPrivateAccess = "true"))
    float ComboInputWindow = 0.3f;

    // Currency configuration
    UPROPERTY(Transient, BlueprintReadOnly, Category = "Currency")
    int CurrentCurrency = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
    int MaxCurrency = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
    int MinimumOverflowCrystalCount = 1;

    // Runtime state
    UPROPERTY(Transient)
    TObjectPtr<APlayerController> CachedPlayerController;

    // Combo system state
    bool bComboInputBuffered = false;
    FTimerHandle ComboWindowTimerHandle;

    // Transient arrays
    UPROPERTY(Transient)
    TArray<AActor*> ActorsOverlappingPreview;

    UPROPERTY(Transient)
    TArray<AActor*> ActorsOverlappingCurrencyCollector;

private:
    // Injected Dependencies
    UPROPERTY(Transient)
    UStructurePlacementManager* PlacementSubsystem;

    UPROPERTY(Transient)
    UEntityManager* EntityManager;

    UPROPERTY(Transient)
    UDebugInformationManager* DebugManager;

    // Structure placement state
    EPreviewStructurePlacementState StructurePlacementState = EPreviewStructurePlacementState::None;
    EStructurePlacementValidityState StructurePlacementValidityState =
        EStructurePlacementValidityState::None;
    EStructurePlacementInvalidityReason StructurePlacementInvalidityReason =
        EStructurePlacementInvalidityReason::None;

    FVector StructurePlacementLocation;
    FRotator StructurePlacementRotation;
    FHitResult PreviewRaycastHitResult;

    UPROPERTY(Transient)
    TObjectPtr<ADefensiveStructure> StructurePreview;

    // Root motion blending configuration
    UPROPERTY(EditAnywhere, Category = "Root Motion", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float RootMotionBlendInDuration = 0.3f;

    UPROPERTY(EditAnywhere, Category = "Root Motion")
    UCurveFloat* RootMotionBlendCurve = nullptr;

    // Character rotation configuration
    UPROPERTY(EditAnywhere,
        Category = "Character Rotation",
        meta = (ClampMin = "0.0", ClampMax = "3600.0"))
    float NormalRotationSpeed = 540.0f; // degrees per second for normal movement

    UPROPERTY(EditAnywhere, Category = "Character Rotation", meta = (ClampMin = "0.0"))
    float MovementThreshold = 10.0f; // minimum velocity to trigger rotation

    // Attack rotation burst configuration
    UPROPERTY(EditAnywhere,
        Category = "Attack Rotation",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AttackRotationDuration = 0.2f; // how long the rotation burst lasts

    UPROPERTY(EditAnywhere,
        Category = "Attack Rotation",
        meta = (ClampMin = "0.0", ClampMax = "3600.0"))
    float AttackRotationSpeed = 1800.0f; // initial rotation speed that fades to zero

    UPROPERTY(EditAnywhere, Category = "Attack Rotation")
    UCurveFloat* AttackRotationCurve = nullptr; // curve for rotation falloff

    // Runtime blending state
    FVector InitialAttackVelocity;
    float AttackStartTime = 0.0f;
    bool bShouldBlendRootMotionForAttack = false;

    // Attack rotation state
    FVector LastMovementInput;     // track input direction separately from velocity
    FRotator AttackTargetRotation; // where to rotate toward during attack burst
    bool bIsPerformingAttackRotation = false;
};
