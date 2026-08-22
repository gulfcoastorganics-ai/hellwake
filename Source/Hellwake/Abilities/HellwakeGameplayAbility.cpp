#include "Abilities/HellwakeGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Kismet/GameplayStatics.h"
#include "HellwakeGameplayTags.h"

UHellwakeGameplayAbility::UHellwakeGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationBlockedTags.AddTag(HellwakeTags::State_Dead);
	ActivationBlockedTags.AddTag(HellwakeTags::State_Cinematic);
}

int32 UHellwakeGameplayAbility::ApplyRadialDamage(const FVector& Center, float RadiusCm, float MinDamage, float MaxDamage,
	TSubclassOf<UGameplayEffect> DamageEffectClass, bool bAlwaysCrit)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (!Avatar || !SourceASC || !DamageEffectClass)
	{
		return 0;
	}

	TArray<AActor*> Overlapped;
	TArray<AActor*> Ignore{ Avatar };
	UGameplayStatics::SphereOverlapActors(Avatar, Center, RadiusCm,
		TArray<TEnumAsByte<EObjectTypeQuery>>{ UEngineTypes::ConvertToObjectType(ECC_Pawn) }, nullptr, Ignore, Overlapped);

	int32 HitCount = 0;
	for (AActor* Target : Overlapped)
	{
		if (!IsValid(Target) || Target == Avatar)
		{
			continue;
		}

		IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(Target);
		UAbilitySystemComponent* TargetASC = TargetASI ? TargetASI->GetAbilitySystemComponent() : nullptr;
		if (!TargetASC)
		{
			continue;
		}

		if (FVector::Dist(Target->GetActorLocation(), Center) > RadiusCm)
		{
			continue;
		}

		const float Damage = FMath::FRandRange(MinDamage, MaxDamage);
		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddSourceObject(this);
		FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), Context);
		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), Damage);
			if (bAlwaysCrit)
			{
				Spec.Data->DynamicGrantedTags.AddTag(HellwakeTags::Event_Damage_Crit);
			}
			SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data, TargetASC);
			++HitCount;
		}
	}

	return HitCount;
}
