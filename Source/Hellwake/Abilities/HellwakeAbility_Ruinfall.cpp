#include "Abilities/HellwakeAbility_Ruinfall.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Camera/HellwakeCameraDirector.h"
#include "GameplayEffects/HellwakeGE_Damage.h"
#include "GameplayEffects/HellwakeGE_AbilityRuntime.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "HellwakeGameplayTags.h"

UHellwakeAbility_Ruinfall::UHellwakeAbility_Ruinfall()
{
	AbilityTags.AddTag(HellwakeTags::Ability_Ruinfall);
	DamageEffectClass = UHellwakeGE_Damage::StaticClass();
	CooldownGameplayEffectClass = UHellwakeGE_Cooldown_Ruinfall::StaticClass();
	CostGameplayEffectClass = UHellwakeGE_Cost_Ruinfall::StaticClass();
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
	DrawDebugCircle(GetWorld(), CapturedOrigin + FVector(0.f, 0.f, 8.f), RadiusCm, 64, FColor::Purple,
		false, TelegraphSeconds, 0, 7.f, FVector(1.f, 0.f, 0.f), FVector(0.f, 1.f, 0.f), false);

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
	DrawDebugSphere(GetWorld(), CapturedOrigin + FVector(0.f, 0.f, 80.f), RadiusCm * 0.45f, 20, FColor::Purple, false, 0.22f, 0, 8.f);

	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		if (UHellwakeCameraDirector* Director = Avatar->FindComponentByClass<UHellwakeCameraDirector>())
		{
			Director->TriggerShake(0.7f);
		}
	}

	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
}
