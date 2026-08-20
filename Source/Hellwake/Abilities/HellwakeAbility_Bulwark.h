// F. Prototype ability('W' key, mapped to F): cost 12 Wrath, cd 12s, 10s
// self-buff setting IncomingDamageMultiplier to 0.45 (i.e. -55% incoming
// damage), grants a "FORT 10s" HUD status pip (Status.Fortified).
#pragma once

#include "CoreMinimal.h"
#include "Abilities/HellwakeGameplayAbility.h"
#include "HellwakeAbility_Bulwark.generated.h"

class UGameplayEffect;

UCLASS()
class HELLWAKE_API UHellwakeAbility_Bulwark : public UHellwakeGameplayAbility
{
	GENERATED_BODY()

public:
	UHellwakeAbility_Bulwark();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	// GE_Hellwake_Bulwark: duration 10s, sets IncomingDamageMultiplier to
	// 0.45 for the duration (Override op), grants Status.Fortified.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Bulwark")
	TSubclassOf<UGameplayEffect> BulwarkEffectClass;
};
