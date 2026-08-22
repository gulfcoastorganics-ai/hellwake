#include "Abilities/HellwakeAbility_WakeOfHell.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Camera/HellwakeCameraDirector.h"
#include "GameplayEffects/HellwakeGE_Damage.h"
#include "GameplayEffects/HellwakeGE_CinematicLock.h"
#include "GameplayEffects/HellwakeGE_AbilityRuntime.h"
#include "Player/HellwakeCharacter.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "HellwakeGameplayTags.h"

UHellwakeAbility_WakeOfHell::UHellwakeAbility_WakeOfHell()
{
	AbilityTags.AddTag(HellwakeTags::Ability_WakeOfHell);
	DamageEffectClass = UHellwakeGE_Damage::StaticClass();
	CinematicLockEffectClass = UHellwakeGE_CinematicLock::StaticClass();
	CooldownGameplayEffectClass = UHellwakeGE_Cooldown_WakeOfHell::StaticClass();
}

void UHellwakeAbility_WakeOfHell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* Avatar = GetAvatarActorFromActorInfo();
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!Avatar || !ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (CinematicLockEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(CinematicLockEffectClass, GetAbilityLevel(), Context);
		if (Spec.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
		}
	}
	if (AHellwakeCharacter* Player = Cast<AHellwakeCharacter>(Avatar))
	{
		Player->BeginCinematic(1.6f);
	}

	const FVector Origin = Avatar->GetActorLocation();
	TArray<AActor*> Overlapped;
	TArray<AActor*> Ignore{ Avatar };
	UGameplayStatics::SphereOverlapActors(Avatar, Origin, RadiusCm,
		TArray<TEnumAsByte<EObjectTypeQuery>>{ UEngineTypes::ConvertToObjectType(ECC_Pawn) }, nullptr, Ignore, Overlapped);

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

		const float DistanceMeters = FVector::Dist(Target->GetActorLocation(), Origin) / 100.f;
		if (DistanceMeters * 100.f > RadiusCm)
		{
			continue;
		}

		const float BaseFalloffDamage = FMath::Max(0.f, MaxDamageAtCenter - FalloffPerMeter * DistanceMeters);
		const float Damage = BaseFalloffDamage * FMath::FRandRange(0.9f, 1.1f);
		if (Damage <= 0.f || !DamageEffectClass)
		{
			continue;
		}

		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(this);
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), Context);
		if (Spec.IsValid())
		{
			Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), Damage);
			Spec.Data->DynamicGrantedTags.AddTag(HellwakeTags::Event_Damage_Crit);
			ASC->ApplyGameplayEffectSpecToTarget(*Spec.Data, TargetASC);
		}
	}

	DrawDebugCircle(GetWorld(), Origin + FVector(0.f, 0.f, 10.f), RadiusCm, 96, FColor::Orange,
		false, 0.65f, 0, 12.f, FVector(1.f, 0.f, 0.f), FVector(0.f, 1.f, 0.f), false);
	DrawDebugSphere(GetWorld(), Origin + FVector(0.f, 0.f, 100.f), FMath::Min(RadiusCm * 0.28f, 500.f), 24,
		FColor::Red, false, 0.35f, 0, 10.f);
	if (UHellwakeCameraDirector* Director = Avatar->FindComponentByClass<UHellwakeCameraDirector>())
	{
		Director->TriggerShake(1.2f);
		Director->TriggerHitStop(0.14f);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
