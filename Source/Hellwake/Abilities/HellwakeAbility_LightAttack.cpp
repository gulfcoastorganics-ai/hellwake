#include "Abilities/HellwakeAbility_LightAttack.h"
#include "HellwakeGameplayTags.h"

UHellwakeAbility_LightAttack::UHellwakeAbility_LightAttack()
{
	AbilityTags.AddTag(HellwakeTags::Ability_Attack_Light);
	ReachCm = 340.f;
	ArcHalfAngleDegrees = 63.f;
	BaseDamage = 120.f;
	WrathGainOnHit = 5.f;
	KnockbackImpulse = 50.f;
	// CooldownGameplayEffectClass = GE_Hellwake_Cooldown_LightAttack (0.42s), set in-editor.
}
