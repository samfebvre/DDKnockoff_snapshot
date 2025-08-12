#include "Health/HealthComponent.h"

#include "Health/HealthBarWidget.h"
#include "Health/HealthBarWidgetComponent.h"
#include "Core/DDKnockoffGameSettings.h"
#include "UObject/ConstructorHelpers.h"


// Sets default values for this component's properties
UHealthComponent::UHealthComponent() {
    PrimaryComponentTick.bCanEverTick = false;
    HealthBarWidgetComponent = CreateDefaultSubobject<UHealthBarWidgetComponent>(
        TEXT("HealthBarWidget"));

    // Load health bar widget class from game settings
    const UDDKnockoffGameSettings* GameSettings = UDDKnockoffGameSettings::Get();
    if (GameSettings && !GameSettings->HealthBarWidgetClass.IsEmpty()) {
        static ConstructorHelpers::FClassFinder<UUserWidget> HealthBarWidgetClassFinder(
            *GameSettings->HealthBarWidgetClass);
        if (UClass* WidgetClass = HealthBarWidgetClassFinder.Class) {
            HealthBarWidgetClass = WidgetClass;
        }
    }
}

void UHealthComponent::OnRegister() {
    Super::OnRegister();
    HealthBarWidgetComponent->SetupAttachment(GetOwner()->GetRootComponent());
    HealthBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
    HealthBarWidgetComponent->SetDrawSize(FVector2D(100.f, 10.f));
    HealthBarWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
    HealthBarWidgetComponent->SetCastShadow(false);
    HealthBarWidgetComponent->SetCanEverAffectNavigation(false);
}

// Called when the game starts
void UHealthComponent::BeginPlay() {
    Super::BeginPlay();

    // Validate configuration
    ValidateConfiguration();

    CurrentHealth = MaxHealth;

    if (HealthBarWidgetClass) { HealthBarWidgetComponent->SetWidgetClass(HealthBarWidgetClass); }

    // Set initial visibility based on health state
    UpdateHealthBarVisibility();
}

void UHealthComponent::TakeDamage(float Amount) {
    if (Amount <= 0.0f) { return; }

    CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.0f, MaxHealth);
    UpdateHealthBarFillAmount();

    // Update health bar visibility based on current health
    UpdateHealthBarVisibility();

    if (IsDead()) { OnReachedZeroHealth.Broadcast(); }
}

bool UHealthComponent::IsDead() const { return CurrentHealth <= 0.0f; }

// Not const because making it so breaks the OnHealthChanged add dynamic call
void UHealthComponent::UpdateHealthBarFillAmount() const {
    if (auto* HealthBarWidget = Cast<UHealthBarWidget>(HealthBarWidgetComponent->GetWidget())) {
        HealthBarWidget->UpdateHealthBarFillAmount(CurrentHealth / MaxHealth);
    }
}

void UHealthComponent::SetHealthBarVisibility(bool bVisible) const {
    if (HealthBarWidgetComponent) { HealthBarWidgetComponent->SetVisibility(bVisible); }
}

void UHealthComponent::UpdateHealthBarVisibility() {
    if (bHideHealthBarWhenFullHealth) {
        // Show health bar only when damaged (not at full health)
        const bool bShouldShow = CurrentHealth < MaxHealth && CurrentHealth > 0.0f;
        SetHealthBarVisibility(bShouldShow);
    }
}

// IConfigurationValidatable interface implementation
void UHealthComponent::ValidateConfiguration() const {
    // Validate that MaxHealth is positive
    ensureAlways(MaxHealth > 0.0f);

    // Validate that HealthBarWidgetClass is set
    ensureAlways(HealthBarWidgetClass != nullptr);
}
