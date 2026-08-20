// Shared base for Light and Heavy attack. Reproduces the prototype's
// discovered hit-resolution model exactly (see project-bible/combat.md
// "Targeting" section for why this is a deliberate port, not an oversight):
// an instantaneous facing-cone overlap test on activation, no lock-on,
// no per-target line trace. attack(heavy) in hellwake-game.js:
//
//   const reach = heavy ? 4.4 : 3.4, arc = heavy ? 1.5 : 1.1, base = heavy ? 260 : 120;
//   for (const e of enemies) {
//     const dist = ...; if (dist > reach + e.def.ring * 0.4) continue;
//     const ang = Math.abs(angDiff(atan2(d.x, d.z), P.facing));
//     if (ang > arc) continue;
//     damageEnemy(e, base * (0.85 + Math.random()*0.3) * (crit ? 2.6 : 1), ...);
//   }
//
// `arc` there is a full half-angle in radians already generous (1.1 rad =
// 63deg, 1.5 rad = 86deg) — it reads as wide-cone melee, not a narrow
// hitscan, which is why it still landed hits during play-testing once the
// player was moving/facing roughly at a target. Damage is applied through a
// GameplayEffect (GE_Hellwake_Damage, authored in-editor) via SetByCaller so
// designers can retune numbers without touching C++.
#pragma once

#include "CoreMinimal.h"
#include "Abilities/HellwakeGameplayAbility.h"
#include "HellwakeAbility_MeleeAttack.generated.h"

class UGameplayEffect;

UCLASS(Abstract)
class HELLWAKE_API UHellwakeAbility_MeleeAttack : public UHellwakeGameplayAbility
{
	GENERATED_BODY()

public:
	UHellwakeAbility_MeleeAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// cm. Prototype: 3.4 (light) / 4.4 (heavy) meters.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Melee")
	float ReachCm = 340.f;

	// Half-angle of the facing cone, degrees. Prototype: 1.1 rad / 1.5 rad.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Melee")
	float ArcHalfAngleDegrees = 63.f;

	// Base damage before the 0.85-1.15x roll and crit multiplier.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Melee")
	float BaseDamage = 120.f;

	// Prototype: 27% flat crit chance, x2.6 damage on crit, for every melee
	// swing (light and heavy share the roll).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Melee")
	float CritChance = 0.27f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Melee")
	float CritMultiplier = 2.6f;

	// Wrath gained per landed swing (only if at least one target was hit).
	// Prototype: +5 light / +9 heavy.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Melee")
	float WrathGainOnHit = 5.f;

	// Knockback impulse strength imparted to hit targets, along the
	// target-minus-attacker horizontal vector. Prototype: 0.5 (light) / 1.1
	// (heavy) as a position-add-per-frame scalar, expressed here as cm.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Melee")
	float KnockbackImpulse = 80.f;

	// Applied to each hit target. Should read its magnitude via
	// SetByCaller tag "Data.Damage" (see GE_Hellwake_Damage in
	// project-bible/abilities.md) — this class sets that tag per hit.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Melee")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// How wide (cm) the overlap sweep looks before the per-target angle
	// filter narrows it down to the true facing cone. Should comfortably
	// exceed ReachCm + the largest enemy engagement ring in play.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Melee")
	float OverlapSweepRadiusCm = 1400.f;

private:
	// Shared implementation, called by ActivateAbility with the two
	// subclasses' tunables already set as members.
	void ResolveMeleeHit();
};
