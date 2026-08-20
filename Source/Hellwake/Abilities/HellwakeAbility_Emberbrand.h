// Q. Prototype ability('Q'): cost 18 Wrath, cd 6s, instant cone/AoE burst
// 7.5u radius at a point 5u ahead of the player, ~420-600 dmg (always
// crit-tier per the prototype's damage call), grants a 9s cosmetic "EMBER"
// status pip (Status.Ember here — purely cosmetic in the source, no
// mechanical effect beyond the HUD pip).
#pragma once

#include "CoreMinimal.h"
#include "Abilities/HellwakeGameplayAbility.h"
#include "HellwakeAbility_Emberbrand.generated.h"

class UGameplayEffect;

UCLASS()
class HELLWAKE_API UHellwakeAbility_Emberbrand : public UHellwakeGameplayAbility
{
	GENERATED_BODY()

public:
	UHellwakeAbility_Emberbrand();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Emberbrand")
	float ForwardOffsetCm = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Emberbrand")
	float RadiusCm = 750.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Emberbrand")
	float MinDamage = 420.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Emberbrand")
	float MaxDamage = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Emberbrand")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// Grants Status.Ember for 9s — cosmetic HUD pip only.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Emberbrand")
	TSubclassOf<UGameplayEffect> EmberStatusEffectClass;
};
