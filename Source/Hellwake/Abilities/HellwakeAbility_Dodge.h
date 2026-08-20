// Shift/Space. Prototype dodge(): 1.1s cd, 0.34s i-frames, 26 u/s (2600cm/s)
// impulse along current movement direction (falls back to facing if not
// moving). Grants State.IFrame for the duration so damage-taking code can
// early-out, matching `if (P.iframe > 0 || P.dead) return;` in damagePlayer().
#pragma once

#include "CoreMinimal.h"
#include "Abilities/HellwakeGameplayAbility.h"
#include "HellwakeAbility_Dodge.generated.h"

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

	// GE that grants the State.IFrame tag for IFrameDuration seconds.
	// Authored in-editor as GE_Hellwake_IFrame (duration = SetByCaller or a
	// fixed 0.34s literal is fine since this value rarely changes).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Dodge")
	TSubclassOf<class UGameplayEffect> IFrameEffectClass;

private:
	FTimerHandle EndHandle;
	void FinishDodge();
};
