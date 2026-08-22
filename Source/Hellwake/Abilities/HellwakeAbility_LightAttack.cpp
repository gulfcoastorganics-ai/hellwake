#include "Abilities/HellwakeAbility_LightAttack.h"
#include "GameplayEffects/HellwakeGE_AbilityRuntime.h"
#include "HellwakeGameplayTags.h"

UHellwakeAbility_LightAttack::UHellwakeAbility_LightAttack()
{
	AbilityTags.AddTag(HellwakeTags::Ability_Attack_Light);
	ReachCm = 340.f;
	ArcHalfAngleDegrees = 63.f;
	BaseDamage = 120.f;
	WrathGainOnHit = 5.f;
	KnockbackImpulse = 50.f;
	CooldownGameplayEffectClass = UHellwakeGE_Cooldown_LightAttack::StaticClass();
}
