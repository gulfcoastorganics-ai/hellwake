// LMB. Prototype: reach 3.4, arc 1.1rad, cd 0.42s, base 120 dmg, +5 Wrath/hit.
#pragma once

#include "CoreMinimal.h"
#include "Abilities/HellwakeAbility_MeleeAttack.h"
#include "HellwakeAbility_LightAttack.generated.h"

UCLASS()
class HELLWAKE_API UHellwakeAbility_LightAttack : public UHellwakeAbility_MeleeAttack
{
	GENERATED_BODY()

public:
	UHellwakeAbility_LightAttack();
};
