// Base GameplayAbility for every Hellwake ability (player and boss alike).
// Centralizes the two gates every prototype ability checked by hand:
//   if (P.dead || cine > 0) return;         -> ActivationBlockedTags
//   if (cds[key] > 0) return;                -> inherited CooldownGameplayEffectClass
// Wrath cost uses the inherited CostGameplayEffectClass exactly like stock
// GAS samples; concrete abilities just set both TSubclassOf in their
// constructor (or a Blueprint child sets them) and call CommitAbility().
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "HellwakeGameplayAbility.generated.h"

UCLASS(Abstract)
class HELLWAKE_API UHellwakeGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UHellwakeGameplayAbility();

protected:
	// Shared AoE helper used by Emberbrand, Ruinfall, and Wake of Hell: finds
	// every pawn with an ASC inside RadiusCm of Center (excluding the
	// avatar), rolls damage in [MinDamage, MaxDamage], and applies
	// DamageEffectClass via SetByCaller tag "Data.Damage". Returns the
	// number of targets hit.
	int32 ApplyRadialDamage(const FVector& Center, float RadiusCm, float MinDamage, float MaxDamage,
		TSubclassOf<UGameplayEffect> DamageEffectClass, bool bAlwaysCrit = false);
};
