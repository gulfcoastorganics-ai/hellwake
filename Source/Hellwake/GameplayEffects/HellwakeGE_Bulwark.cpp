#include "GameplayEffects/HellwakeGE_Bulwark.h"
#include "Attributes/HellwakeAttributeSet.h"
#include "HellwakeGameplayTags.h"

UHellwakeGE_Bulwark::UHellwakeGE_Bulwark()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(10.f));

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UHellwakeAttributeSet::GetIncomingDamageMultiplierAttribute();
	Modifier.ModifierOp = EGameplayModOp::Override;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(0.45f));
	Modifiers.Add(Modifier);

	InheritableOwnedTagsContainer.AddTag(HellwakeTags::Status_Fortified);
}
