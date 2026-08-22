#include "Abilities/HellwakeAbility_HeavyAttack.h"
#include "GameplayEffects/HellwakeGE_AbilityRuntime.h"
#include "HellwakeGameplayTags.h"

UHellwakeAbility_HeavyAttack::UHellwakeAbility_HeavyAttack()
{
	AbilityTags.AddTag(HellwakeTags::Ability_Attack_Heavy);
	ReachCm = 440.f;
	ArcHalfAngleDegrees = 86.f;
	BaseDamage = 260.f;
	WrathGainOnHit = 9.f;
	KnockbackImpulse = 130.f;
	CooldownGameplayEffectClass = UHellwakeGE_Cooldown_HeavyAttack::StaticClass();
}
