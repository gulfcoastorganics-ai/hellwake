#include "Abilities/HellwakeAbility_MeleeAttack.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Attributes/HellwakeAttributeSet.h"
#include "Enemies/HellwakeEnemyBase.h"
#include "GameplayEffects/HellwakeGE_Damage.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HellwakeGameplayTags.h"
#include "Hellwake.h"

UHellwakeAbility_MeleeAttack::UHellwakeAbility_MeleeAttack()
{
	DamageEffectClass = UHellwakeGE_Damage::StaticClass();
}

void UHellwakeAbility_MeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ResolveMeleeHit();
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void UHellwakeAbility_MeleeAttack::ResolveMeleeHit()
{
	ACharacter* AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!AvatarCharacter)
	{
		return;
	}

	const FVector Origin = AvatarCharacter->GetActorLocation();
	const FVector Forward = AvatarCharacter->GetActorForwardVector();

	TArray<AActor*> Overlapped;
	TArray<AActor*> Ignore;
	Ignore.Add(AvatarCharacter);
	UGameplayStatics::SphereOverlapActors(AvatarCharacter, Origin, OverlapSweepRadiusCm,
		TArray<TEnumAsByte<EObjectTypeQuery>>{ UEngineTypes::ConvertToObjectType(ECC_Pawn) }, nullptr, Ignore, Overlapped);

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	bool bHitAnything = false;

	for (AActor* Target : Overlapped)
	{
		if (!IsValid(Target) || Target == AvatarCharacter)
		{
			continue;
		}

		IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(Target);
		UAbilitySystemComponent* TargetASC = TargetASI ? TargetASI->GetAbilitySystemComponent() : nullptr;
		if (!TargetASC)
		{
			continue;
		}

		FVector ToTarget = Target->GetActorLocation() - Origin;
		const float DistanceCm = ToTarget.Size2D();
		const AHellwakeEnemyBase* EnemyTarget = Cast<AHellwakeEnemyBase>(Target);
		const float TargetRadiusFudge = EnemyTarget ? EnemyTarget->GetEngagementRingCm() * 0.4f : 0.f;
		if (DistanceCm > ReachCm + TargetRadiusFudge)
		{
			continue;
		}

		ToTarget.Z = 0.f;
		if (!ToTarget.Normalize())
		{
			continue;
		}

		FVector FlatForward = Forward;
		FlatForward.Z = 0.f;
		if (!FlatForward.Normalize())
		{
			continue;
		}

		const float Dot = FMath::Clamp(FVector::DotProduct(FlatForward, ToTarget), -1.f, 1.f);
		const float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(Dot));
		if (AngleDegrees > ArcHalfAngleDegrees)
		{
			continue;
		}

		const bool bCrit = FMath::FRand() < CritChance;
		const float Roll = FMath::FRandRange(0.85f, 1.15f);
		const float Damage = BaseDamage * Roll * (bCrit ? CritMultiplier : 1.f);

		if (DamageEffectClass && SourceASC)
		{
			FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
			Context.AddSourceObject(this);
			FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), Context);
			if (Spec.IsValid())
			{
				Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), Damage);
				if (bCrit)
				{
					Spec.Data->DynamicGrantedTags.AddTag(HellwakeTags::Event_Damage_Crit);
				}
				SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data, TargetASC);
			}
		}

		if (KnockbackImpulse > 0.f)
		{
			if (ACharacter* TargetCharacter = Cast<ACharacter>(Target))
			{
				FVector Launch = ToTarget * KnockbackImpulse;
				Launch.Z = 40.f;
				TargetCharacter->LaunchCharacter(Launch, true, false);
			}
		}

		bHitAnything = true;
	}

	if (bHitAnything && WrathGainOnHit > 0.f && SourceASC)
	{
		const UHellwakeAttributeSet* AttrSet = SourceASC->GetSet<UHellwakeAttributeSet>();
		if (AttrSet)
		{
			const float NewWrath = FMath::Min(AttrSet->GetMaxWrath(), AttrSet->GetWrath() + WrathGainOnHit);
			SourceASC->ApplyModToAttribute(UHellwakeAttributeSet::GetWrathAttribute(), EGameplayModOp::Override, NewWrath);
		}
	}

	UE_LOG(LogHellwakeCombat, Verbose, TEXT("%s melee swing resolved, hit anything: %s"),
		*AvatarCharacter->GetName(), bHitAnything ? TEXT("true") : TEXT("false"));
}
