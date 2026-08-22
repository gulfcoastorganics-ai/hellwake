#include "Abilities/HellwakeAbility_Emberbrand.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffects/HellwakeGE_Damage.h"
#include "GameplayEffects/HellwakeGE_AbilityRuntime.h"
#include "GameFramework/Actor.h"
#include "HellwakeGameplayTags.h"

UHellwakeAbility_Emberbrand::UHellwakeAbility_Emberbrand()
{
	AbilityTags.AddTag(HellwakeTags::Ability_Emberbrand);
	DamageEffectClass = UHellwakeGE_Damage::StaticClass();
	CooldownGameplayEffectClass = UHellwakeGE_Cooldown_Emberbrand::StaticClass();
	CostGameplayEffectClass = UHellwakeGE_Cost_Emberbrand::StaticClass();
	EmberStatusEffectClass = UHellwakeGE_Status_Ember::StaticClass();
}

void UHellwakeAbility_Emberbrand::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* Avatar = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (Avatar)
	{
		const FVector Origin = Avatar->GetActorLocation() + Avatar->GetActorForwardVector() * ForwardOffsetCm;
		ApplyRadialDamage(Origin, RadiusCm, MinDamage, MaxDamage, DamageEffectClass, true);
	}

	if (ASC && EmberStatusEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(EmberStatusEffectClass, GetAbilityLevel(), Context);
		if (Spec.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
