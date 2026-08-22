#include "Abilities/HellwakeAbility_Emberbrand.h"
#include "AbilitySystemComponent.h"
#include "Camera/HellwakeCameraDirector.h"
#include "DrawDebugHelpers.h"
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
		const int32 HitCount = ApplyRadialDamage(Origin, RadiusCm, MinDamage, MaxDamage, DamageEffectClass, true);

		DrawDebugCircle(GetWorld(), Origin + FVector(0.f, 0.f, 10.f), RadiusCm, 64, FColor::Orange,
			false, 0.42f, 0, 9.f, FVector(1.f, 0.f, 0.f), FVector(0.f, 1.f, 0.f), false);
		DrawDebugSphere(GetWorld(), Origin + FVector(0.f, 0.f, 90.f), 180.f, 18, FColor::Orange, false, 0.24f, 0, 8.f);

		if (UHellwakeCameraDirector* Director = Avatar->FindComponentByClass<UHellwakeCameraDirector>())
		{
			Director->TriggerShake(HitCount > 0 ? 0.5f : 0.3f);
		}
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
