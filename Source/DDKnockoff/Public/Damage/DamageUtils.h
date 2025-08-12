#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Damage/DamagePayload.h"
#include "Damage/DDDamageType.h"
#include "DamageUtils.generated.h"

/**
 * Static utility library for damage application throughout the game.
 * Eliminates repeated faction checking, knockback calculation, and damage payload creation.
 */
UCLASS()
class DDKNOCKOFF_API UDamageUtils : public UBlueprintFunctionLibrary {
    GENERATED_BODY()

public:
    // Core damage application methods

    /**
     * Apply damage to a target actor with faction checking and validation.
     * Knockback will be applied if KnockbackStrength > 0.
     * @param Target - Actor to damage
     * @param Instigator - Actor causing the damage
     * @param DamageAmount - Amount of damage to apply
     * @param KnockbackStrength - Force of the knockback (0 for no knockback)
     * @param DamageType - Type of damage being applied
     * @return true if damage was applied, false if target was invalid or same faction
     */
    UFUNCTION(BlueprintCallable,
        Category = "Damage",
        meta = (Keywords = "damage apply hurt knockback"))
    static bool ApplyDamage(AActor* Target,
                            AActor* Instigator,
                            float DamageAmount,
                            float KnockbackStrength = 0.0f,
                            EDDDamageType DamageType = EDDDamageType::None);

    // Utility methods for validation and calculation

    /**
     * Check if one actor can damage another (faction checking, null checking, self-damage protection).
     * @param Target - Actor that would receive damage
     * @param Instigator - Actor that would cause damage
     * @return true if damage can be applied, false if not allowed
     */
    UFUNCTION(BlueprintCallable,
        Category = "Damage",
        meta = (Keywords = "damage valid check faction"))
    static bool CanDamageTarget(AActor* Target, AActor* Instigator);

    /**
     * Check if two actors are of different factions.
     * @param Actor1 - First actor to check
     * @param Actor2 - Second actor to check
     * @return true if actors are different factions, false if same faction or either is invalid
     */
    UFUNCTION(BlueprintCallable, Category = "Damage", meta = (Keywords = "faction different enemy"))
    static bool AreDifferentFactions(AActor* Actor1, AActor* Actor2);

    /**
     * Calculate standard knockback direction from source to target with optional upward component.
     * @param Target - Actor being knocked back
     * @param Source - Actor causing the knockback
     * @param UpwardComponent - Amount of upward force to add (0.0-1.0 recommended)
     * @return Normalized direction vector for knockback
     */
    UFUNCTION(BlueprintCallable,
        Category = "Damage",
        meta = (Keywords = "knockback direction calculate"))
    static FVector CalculateKnockbackDirection(AActor* Target,
                                               AActor* Source,
                                               float UpwardComponent = 0.2f);

    /**
     * Create a damage payload structure with the provided parameters.
     * @param DamageAmount - Amount of damage
     * @param Instigator - Actor causing the damage
     * @param KnockbackStrength - Force of the knockback (0 for no knockback)
     * @param DamageType - Type of damage
     * @return Configured damage payload structure
     */
    UFUNCTION(BlueprintCallable,
        Category = "Damage",
        meta = (Keywords = "damage payload create knockback"))
    static FDamagePayload CreateDamagePayload(float DamageAmount,
                                              AActor* Instigator,
                                              float KnockbackStrength = 0.0f,
                                              EDDDamageType DamageType = EDDDamageType::None);

private:
    // Internal implementation methods
    static bool ApplyDamageInternal(AActor* Target, const FDamagePayload& DamagePayload);
    static bool ApplyKnockbackInternal(AActor* Target, const FVector& Direction, float Strength);
};
