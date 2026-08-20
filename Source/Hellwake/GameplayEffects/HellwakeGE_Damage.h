// Instant effect: adds a SetByCaller("Data.Damage") amount to the Damage
// meta-attribute. HellwakeAttributeSet::PostGameplayEffectExecute converts
// that into a Health reduction (scaled by IncomingDamageMultiplier) and
// fires Event.Damage.Taken / Event.Death. Every source of damage in the
// game (melee, Emberbrand, Ruinfall, Wake of Hell, enemy/boss attacks) uses
// this same effect class — only the SetByCaller magnitude differs per hit.
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "HellwakeGE_Damage.generated.h"

UCLASS()
class HELLWAKE_API UHellwakeGE_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UHellwakeGE_Damage();
};
