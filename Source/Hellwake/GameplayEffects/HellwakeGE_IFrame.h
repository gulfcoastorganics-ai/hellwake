// Duration effect, 0.34s. Grants State.IFrame. Prototype: P.iframe = 0.34
// in dodge() — `if (P.iframe > 0 || P.dead) return;` in damagePlayer().
// Keep Duration in sync with UHellwakeAbility_Dodge::IFrameDuration.
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "HellwakeGE_IFrame.generated.h"

UCLASS()
class HELLWAKE_API UHellwakeGE_IFrame : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UHellwakeGE_IFrame();
};
