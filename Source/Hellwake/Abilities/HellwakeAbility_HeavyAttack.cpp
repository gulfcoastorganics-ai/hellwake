#include "Abilities/HellwakeAbility_HeavyAttack.h"
#include "HellwakeGameplayTags.h"

UHellwakeAbility_HeavyAttack::UHellwakeAbility_HeavyAttack()
{
	AbilityTags.AddTag(HellwakeTags::Ability_Attack_Heavy);
	ReachCm = 440.f;
	ArcHalfAngleDegrees = 86.f;
	BaseDamage = 260.f;
	WrathGainOnHit = 9.f;
	KnockbackImpulse = 130.f;
	// CooldownGameplayEffectClass = GE_Hellwake_Cooldown_HeavyAttack (0.924s), set in-editor.
}
