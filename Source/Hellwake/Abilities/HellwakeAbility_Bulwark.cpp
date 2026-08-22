#include "Abilities/HellwakeAbility_Bulwark.h"
#include "AbilitySystemComponent.h"
#include "Camera/HellwakeCameraDirector.h"
#include "DrawDebugHelpers.h"
#include "GameplayEffects/HellwakeGE_Bulwark.h"
#include "GameplayEffects/HellwakeGE_AbilityRuntime.h"
#include "GameFramework/Actor.h"
#include "HellwakeGameplayTags.h"

UHellwakeAbility_Bulwark::UHellwakeAbility_Bulwark()
{
	AbilityTags.AddTag(HellwakeTags::Ability_Bulwark);
	BulwarkEffectClass = UHellwakeGE_Bulwark::StaticClass();
	CooldownGameplayEffectClass = UHellwakeGE_Cooldown_Bulwark::StaticClass();
	CostGameplayEffectClass = UHellwakeGE_Cost_Bulwark::StaticClass();
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

	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		const FVector Origin = Avatar->GetActorLocation();
		DrawDebugSphere(GetWorld(), Origin + FVector(0.f, 0.f, 95.f), 190.f, 20, FColor::Cyan, false, 0.45f, 0, 7.f);
		DrawDebugCircle(GetWorld(), Origin + FVector(0.f, 0.f, 10.f), 450.f, 48, FColor::Cyan,
			false, 0.55f, 0, 7.f, FVector(1.f, 0.f, 0.f), FVector(0.f, 1.f, 0.f), false);
		if (UHellwakeCameraDirector* Director = Avatar->FindComponentByClass<UHellwakeCameraDirector>())
		{
			Director->TriggerShake(0.22f);
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
