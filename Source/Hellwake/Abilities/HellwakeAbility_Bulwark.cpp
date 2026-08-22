#include "Abilities/HellwakeAbility_Bulwark.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffects/HellwakeGE_Bulwark.h"
#include "HellwakeGameplayTags.h"

UHellwakeAbility_Bulwark::UHellwakeAbility_Bulwark()
{
	AbilityTags.AddTag(HellwakeTags::Ability_Bulwark);
	BulwarkEffectClass = UHellwakeGE_Bulwark::StaticClass();
}

void UHellwakeAbility_Bulwark::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (BulwarkEffectClass)
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(BulwarkEffectClass, GetAbilityLevel(), Context);
			if (Spec.IsValid())
			{
				ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
			}
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
