// Instant effect applied once in HellwakeCharacter::PossessedBy. Sets the
// starting attribute block. Prototype defaults (hellwake-game.js):
//   P = { hp:100, hpMax:100, wrath:100, wrathMax:100, ... }
//   HealthRegenRate 1.6/s, WrathRegenRate 4.5/s (see PassiveRegen GE).
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "HellwakeGE_PlayerDefaults.generated.h"

UCLASS()
class HELLWAKE_API UHellwakeGE_PlayerDefaults : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UHellwakeGE_PlayerDefaults();
};
