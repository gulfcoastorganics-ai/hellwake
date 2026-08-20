#include "Abilities/HellwakeAbility_Ruinfall.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "GameFramework/Actor.h"
#include "HellwakeGameplayTags.h"

UHellwakeAbility_Ruinfall::UHellwakeAbility_Ruinfall()
{
	AbilityTags.AddTag(HellwakeTags::Ability_Ruinfall);
}

void UHellwakeAbility_Ruinfall::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Captured once, at cast time — the prototype does not retarget this
	// point if the player or enemies move during the telegraph window.
	CapturedOrigin = Avatar->GetActorLocation() + Avatar->GetActorForwardVector() * ForwardOffsetCm;

	// TODO(VFX): telegraph(px, pz, 5.5, 0.75, ...) -> spawn a ring decal /
	// Niagara telegraph at CapturedOrigin now, sized to RadiusCm, that fills
	// in visually over TelegraphSeconds so enemies/players can read and
	// dodge it before OnTelegraphComplete detonates.

	DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, TelegraphSeconds);
	DelayTask->OnFinish.AddDynamic(this, &UHellwakeAbility_Ruinfall::OnTelegraphComplete);
	DelayTask->ReadyForActivation();
}

void UHellwakeAbility_Ruinfall::OnTelegraphComplete()
{
	ApplyRadialDamage(CapturedOrigin, RadiusCm, MinDamage, MaxDamage, DamageEffectClass, /*bAlwaysCrit=*/true);

	// TODO(VFX): burst(px,1,pz,0xb79bf0,2,34) + shockwave(px,pz,6.5,0xb79bf0) -> NS_Ruinfall_Detonate.
	// TODO(Camera): shake = max(shake, 0.7).

	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
}
