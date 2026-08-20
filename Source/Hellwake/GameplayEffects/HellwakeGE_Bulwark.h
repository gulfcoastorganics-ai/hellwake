// Duration effect, 10s. Overrides IncomingDamageMultiplier to 0.45 and
// grants Status.Fortified for the duration. Prototype: P.bulwark = 10 in
// ability('W' / F key) — `P.bulwark > 0 ? 0.45 : 1` in damagePlayer().
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "HellwakeGE_Bulwark.generated.h"

UCLASS()
class HELLWAKE_API UHellwakeGE_Bulwark : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UHellwakeGE_Bulwark();
};
