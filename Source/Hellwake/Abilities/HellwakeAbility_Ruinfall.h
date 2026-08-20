// E. Prototype ability('E'): cost 22 Wrath, cd 9s. Telegraphs a 5.5u-radius
// ring at a point 5u ahead of the player (captured at cast time, not
// retargeted), waits 0.75s, then detonates for 620-840 dmg to everything
// within 6.5u of that point. The delay is the important part to preserve —
// it's the one ability that gives enemies (and the Gravewarden) a
// dodge-the-telegraph counterplay window; Emberbrand and Wake of Hell are
// both instant.
#pragma once

#include "CoreMinimal.h"
#include "Abilities/HellwakeGameplayAbility.h"
#include "HellwakeAbility_Ruinfall.generated.h"

class UGameplayEffect;
class UAbilityTask_WaitDelay;

UCLASS()
class HELLWAKE_API UHellwakeAbility_Ruinfall : public UHellwakeGameplayAbility
{
	GENERATED_BODY()

public:
	UHellwakeAbility_Ruinfall();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Ruinfall")
	float ForwardOffsetCm = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Ruinfall")
	float TelegraphSeconds = 0.75f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Ruinfall")
	float RadiusCm = 650.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Ruinfall")
	float MinDamage = 620.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Ruinfall")
	float MaxDamage = 840.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Ruinfall")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

private:
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitDelay> DelayTask;

	FVector CapturedOrigin = FVector::ZeroVector;

	UFUNCTION()
	void OnTelegraphComplete();
};
