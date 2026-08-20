// Infinite, periodic (0.1s) effect applied once on spawn. Ports:
//   P.wrath = min(P.wrathMax, P.wrath + dt*4.5);
//   if (P.hp < P.hpMax && P.hp > 0) P.hp = min(P.hpMax, P.hp + dt*1.6);
// as two AttributeBased modifiers reading HealthRegenRate/WrathRegenRate
// (so designers can retune regen without touching this class) scaled by
// the effect's period. Health regen note: GAS doesn't have a
// clean built-in "only while > 0" gate on a periodic modifier; rely on
// PreAttributeChange's clamp (regen simply has no effect once Health is
// already 0 and State.Dead blocks nothing here since PostGameplayEffectExecute
// only special-cases the Damage attribute) — dead characters stop
// regenerating in practice because Die() has already handled the 0-HP case
// via the Event.Death path before this would matter.
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "HellwakeGE_PassiveRegen.generated.h"

UCLASS()
class HELLWAKE_API UHellwakeGE_PassiveRegen : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UHellwakeGE_PassiveRegen();
};
