#include "GameplayEffects/HellwakeGE_AbilityRuntime.h"
#include "Attributes/HellwakeAttributeSet.h"
#include "HellwakeGameplayTags.h"

namespace
{
	void ConfigureCooldown(UGameplayEffect& Effect, float Seconds, const FGameplayTag& CooldownTag)
	{
		Effect.DurationPolicy = EGameplayEffectDurationType::HasDuration;
		Effect.DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(Seconds));
		Effect.InheritableOwnedTagsContainer.AddTag(CooldownTag);
	}

	void ConfigureWrathCost(UGameplayEffect& Effect, float Cost)
	{
		Effect.DurationPolicy = EGameplayEffectDurationType::Instant;

		FGameplayModifierInfo Modifier;
		Modifier.Attribute = UHellwakeAttributeSet::GetWrathAttribute();
		Modifier.ModifierOp = EGameplayModOp::Additive;
		Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-FMath::Abs(Cost)));
		Effect.Modifiers.Add(Modifier);
	}
}

UHellwakeGE_Cooldown_LightAttack::UHellwakeGE_Cooldown_LightAttack()
{
	ConfigureCooldown(*this, 0.42f, HellwakeTags::Cooldown_LightAttack);
}

UHellwakeGE_Cooldown_HeavyAttack::UHellwakeGE_Cooldown_HeavyAttack()
{
	ConfigureCooldown(*this, 0.924f, HellwakeTags::Cooldown_HeavyAttack);
}

UHellwakeGE_Cooldown_Dodge::UHellwakeGE_Cooldown_Dodge()
{
	ConfigureCooldown(*this, 1.1f, HellwakeTags::Cooldown_Dodge);
}

UHellwakeGE_Cooldown_Emberbrand::UHellwakeGE_Cooldown_Emberbrand()
{
	ConfigureCooldown(*this, 6.f, HellwakeTags::Cooldown_Emberbrand);
}

UHellwakeGE_Cooldown_Bulwark::UHellwakeGE_Cooldown_Bulwark()
{
	ConfigureCooldown(*this, 12.f, HellwakeTags::Cooldown_Bulwark);
}

UHellwakeGE_Cooldown_Ruinfall::UHellwakeGE_Cooldown_Ruinfall()
{
	ConfigureCooldown(*this, 9.f, HellwakeTags::Cooldown_Ruinfall);
}

UHellwakeGE_Cooldown_WakeOfHell::UHellwakeGE_Cooldown_WakeOfHell()
{
	ConfigureCooldown(*this, 40.f, HellwakeTags::Cooldown_WakeOfHell);
}

UHellwakeGE_Cost_Emberbrand::UHellwakeGE_Cost_Emberbrand()
{
	ConfigureWrathCost(*this, 18.f);
}

UHellwakeGE_Cost_Bulwark::UHellwakeGE_Cost_Bulwark()
{
	ConfigureWrathCost(*this, 12.f);
}

UHellwakeGE_Cost_Ruinfall::UHellwakeGE_Cost_Ruinfall()
{
	ConfigureWrathCost(*this, 22.f);
}

UHellwakeGE_Status_Ember::UHellwakeGE_Status_Ember()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(9.f));
	InheritableOwnedTagsContainer.AddTag(HellwakeTags::Status_Ember);
}
