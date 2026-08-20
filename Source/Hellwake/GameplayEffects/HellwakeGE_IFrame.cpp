#include "GameplayEffects/HellwakeGE_IFrame.h"
#include "HellwakeGameplayTags.h"

UHellwakeGE_IFrame::UHellwakeGE_IFrame()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(0.34f));
	InheritableOwnedTagsContainer.AddTag(HellwakeTags::State_IFrame);
}
