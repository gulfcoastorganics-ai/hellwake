#include "GameplayEffects/HellwakeGE_Cooldown.h"

UHellwakeGE_Cooldown::UHellwakeGE_Cooldown()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;
	// Subclasses (Blueprint children) set DurationMagnitude to a literal
	// ScalableFloat matching the ability's cooldown, and add exactly one
	// tag to InheritableOwnedTagsContainer (e.g. "Cooldown.Emberbrand")
	// which the owning ability's CooldownTags checks against.
}
