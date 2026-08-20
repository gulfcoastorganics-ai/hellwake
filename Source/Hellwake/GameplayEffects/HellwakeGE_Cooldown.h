// Generic reusable cooldown base. Each ability's CooldownGameplayEffectClass
// points at a Blueprint child of this class with CooldownDuration and
// CooldownTag set per-ability in the Class Defaults panel — this is the
// standard GAS cooldown pattern (see Epic's Lyra/ActionRPG samples) and
// covers all seven Hellwake abilities without seven C++ classes:
//
//   GE_Cooldown_LightAttack   : Duration 0.42,  Tag Cooldown.LightAttack
//   GE_Cooldown_HeavyAttack   : Duration 0.924, Tag Cooldown.HeavyAttack
//   GE_Cooldown_Dodge         : Duration 1.1,   Tag Cooldown.Dodge
//   GE_Cooldown_Emberbrand    : Duration 6.0,   Tag Cooldown.Emberbrand
//   GE_Cooldown_Bulwark       : Duration 12.0,  Tag Cooldown.Bulwark
//   GE_Cooldown_Ruinfall      : Duration 9.0,   Tag Cooldown.Ruinfall
//   GE_Cooldown_WakeOfHell    : Duration 40.0,  Tag Cooldown.WakeOfHell
//
// Each ability's AbilityTags should include the matching Cooldown.* tag via
// its CooldownGameplayEffectClass's InheritableOwnedTagsContainer, and the
// ability itself lists that tag in a private "cooldown check" the engine
// already does automatically from CooldownGameplayEffectClass — no extra
// C++ needed once the Blueprint children are authored.
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "HellwakeGE_Cooldown.generated.h"

UCLASS(Abstract)
class HELLWAKE_API UHellwakeGE_Cooldown : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UHellwakeGE_Cooldown();
};
