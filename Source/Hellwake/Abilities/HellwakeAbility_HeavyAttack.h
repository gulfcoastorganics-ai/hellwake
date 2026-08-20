// RMB. Prototype: reach 4.4, arc 1.5rad, cd 0.42*2.2=0.924s, base 260 dmg, +9 Wrath/hit.
#pragma once

#include "CoreMinimal.h"
#include "Abilities/HellwakeAbility_MeleeAttack.h"
#include "HellwakeAbility_HeavyAttack.generated.h"

UCLASS()
class HELLWAKE_API UHellwakeAbility_HeavyAttack : public UHellwakeAbility_MeleeAttack
{
	GENERATED_BODY()

public:
	UHellwakeAbility_HeavyAttack();
};
