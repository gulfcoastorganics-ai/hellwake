#include "Abilities/HellwakeAbility_Ruinfall.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "GameplayEffects/HellwakeGE_Damage.h"
#include "GameFramework/Actor.h"
#include "HellwakeGameplayTags.h"

UHellwakeAbility_Ruinfall::UHellwakeAbility_Ruinfall()
{
	AbilityTags.AddTag(HellwakeTags::Ability_Ruinfall);
	DamageEffectClass = UHellwakeGE_Damage::StaticClass();
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

	CapturedOrigin = Avatar->GetActorLocation() + Avatar->GetActorForwardVector() * ForwardOffsetCm;

	DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, TelegraphSeconds);
	if (!DelayTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	DelayTask->OnFinish.AddDynamic(this, &UHellwakeAbility_Ruinfall::OnTelegraphComplete);
	DelayTask->ReadyForActivation();
}

void UHellwakeAbility_Ruinfall::OnTelegraphComplete()
{
	ApplyRadialDamage(CapturedOrigin, RadiusCm, MinDamage, MaxDamage, DamageEffectClass, true);
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
}
