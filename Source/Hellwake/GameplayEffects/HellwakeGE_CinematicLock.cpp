#include "GameplayEffects/HellwakeGE_CinematicLock.h"
#include "HellwakeGameplayTags.h"

UHellwakeGE_CinematicLock::UHellwakeGE_CinematicLock()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(1.6f));
	InheritableOwnedTagsContainer.AddTag(HellwakeTags::State_Cinematic);
}
