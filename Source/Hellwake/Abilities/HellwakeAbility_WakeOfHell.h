// R. Prototype ability('R'): free (0 Wrath), cd 40s. Self-centered nova,
// radius 22u, damage falls off linearly with distance: (1500 - 30*d) * roll,
// d in prototype units (meters). Forces a 1.6s cinematic lock (input mostly
// disabled, HUD dims) and the heaviest camera shake/hit-stop in the kit.
// This is the only ability in the slice that locks out player input.
#pragma once

#include "CoreMinimal.h"
#include "Abilities/HellwakeGameplayAbility.h"
#include "HellwakeAbility_WakeOfHell.generated.h"

class UGameplayEffect;

UCLASS()
class HELLWAKE_API UHellwakeAbility_WakeOfHell : public UHellwakeGameplayAbility
{
	GENERATED_BODY()

public:
	UHellwakeAbility_WakeOfHell();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|WakeOfHell")
	float RadiusCm = 2200.f;

	// Damage(d_meters) = MaxDamageAtCenter - FalloffPerMeter * d, floored at 0.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|WakeOfHell")
	float MaxDamageAtCenter = 1500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|WakeOfHell")
	float FalloffPerMeter = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|WakeOfHell")
	float CinematicLockSeconds = 1.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|WakeOfHell")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// Grants State.Cinematic for CinematicLockSeconds — reuses the same tag
	// the boss-intro/death cinematics use, so any ability (including this
	// one) is correctly blocked from re-triggering mid-nova.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|WakeOfHell")
	TSubclassOf<UGameplayEffect> CinematicLockEffectClass;
};
