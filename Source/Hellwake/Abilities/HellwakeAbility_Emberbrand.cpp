#include "Abilities/HellwakeAbility_Emberbrand.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "HellwakeGameplayTags.h"

UHellwakeAbility_Emberbrand::UHellwakeAbility_Emberbrand()
{
	AbilityTags.AddTag(HellwakeTags::Ability_Emberbrand);
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
		ApplyRadialDamage(Origin, RadiusCm, MinDamage, MaxDamage, DamageEffectClass, /*bAlwaysCrit=*/true);

		// TODO(VFX): shockwave(fx,7,0xffa040) + burst(fx,1.2,1.5,30) -> NS_Emberbrand_Burst at Origin.
		// TODO(Camera): shake = max(shake, 0.5) -> camera shake asset, magnitude ~0.5 of the standard curve.
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
