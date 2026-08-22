// Shift/Space. Prototype dodge(): 1.1s cd, 0.34s i-frames, 26 u/s (2600cm/s)
// impulse along current movement direction (falls back to facing if not
// moving). Grants State.IFrame for the duration so damage-taking code can
// early-out, matching `if (P.iframe > 0 || P.dead) return;` in damagePlayer().
#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "Abilities/HellwakeGameplayAbility.h"
#include "HellwakeAbility_Dodge.generated.h"

class UGameplayEffect;

UCLASS()
class HELLWAKE_API UHellwakeAbility_Dodge : public UHellwakeGameplayAbility
{
	GENERATED_BODY()

public:
	UHellwakeAbility_Dodge();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Dodge")
	float IFrameDuration = 0.34f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Dodge")
	float ImpulseCmPerSec = 2600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Dodge")
	TSubclassOf<UGameplayEffect> IFrameEffectClass;

private:
	FTimerHandle EndHandle;
	void FinishDodge();
};
