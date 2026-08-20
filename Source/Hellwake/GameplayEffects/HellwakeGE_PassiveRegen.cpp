#include "GameplayEffects/HellwakeGE_PassiveRegen.h"
#include "Attributes/HellwakeAttributeSet.h"

UHellwakeGE_PassiveRegen::UHellwakeGE_PassiveRegen()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	Period = FScalableFloat(0.1f);
	bExecutePeriodicEffectOnApplication = false;

	auto MakeRegenModifier = [](const FGameplayAttribute& TargetAttribute, const FGameplayAttribute& RateAttribute, float PeriodSeconds)
	{
		FGameplayEffectAttributeCaptureDefinition Capture;
		Capture.AttributeToCapture = RateAttribute;
		Capture.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
		Capture.bSnapshot = false;

		FAttributeBasedFloat AttributeBased;
		AttributeBased.BackingAttribute = Capture;
		AttributeBased.AttributeCalculationType = EAttributeBasedFloatCalculationType::AttributeMagnitude;
		AttributeBased.Coefficient = FScalableFloat(PeriodSeconds);

		FGameplayModifierInfo Modifier;
		Modifier.Attribute = TargetAttribute;
		Modifier.ModifierOp = EGameplayModOp::Additive;
		Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(AttributeBased);
		return Modifier;
	};

	// Prototype: dt * 1.6 (Health), dt * 4.5 (Wrath) — Period stands in for dt.
	Modifiers.Add(MakeRegenModifier(UHellwakeAttributeSet::GetHealthAttribute(), UHellwakeAttributeSet::GetHealthRegenRateAttribute(), 0.1f));
	Modifiers.Add(MakeRegenModifier(UHellwakeAttributeSet::GetWrathAttribute(), UHellwakeAttributeSet::GetWrathRegenRateAttribute(), 0.1f));
}
