#include "GameplayEffects/HellwakeGE_PlayerDefaults.h"
#include "Attributes/HellwakeAttributeSet.h"

UHellwakeGE_PlayerDefaults::UHellwakeGE_PlayerDefaults()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	auto MakeOverride = [](const FGameplayAttribute& Attribute, float Value)
	{
		FGameplayModifierInfo Modifier;
		Modifier.Attribute = Attribute;
		Modifier.ModifierOp = EGameplayModOp::Override;
		Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(Value));
		return Modifier;
	};

	Modifiers.Add(MakeOverride(UHellwakeAttributeSet::GetMaxHealthAttribute(), 100.f));
	Modifiers.Add(MakeOverride(UHellwakeAttributeSet::GetHealthAttribute(), 100.f));
	Modifiers.Add(MakeOverride(UHellwakeAttributeSet::GetMaxWrathAttribute(), 100.f));
	Modifiers.Add(MakeOverride(UHellwakeAttributeSet::GetWrathAttribute(), 100.f));
	Modifiers.Add(MakeOverride(UHellwakeAttributeSet::GetHealthRegenRateAttribute(), 1.6f));
	Modifiers.Add(MakeOverride(UHellwakeAttributeSet::GetWrathRegenRateAttribute(), 4.5f));
	Modifiers.Add(MakeOverride(UHellwakeAttributeSet::GetIncomingDamageMultiplierAttribute(), 1.f));
}
