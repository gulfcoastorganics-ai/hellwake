#include "Attributes/HellwakeAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Net/UnrealNetwork.h"
#include "HellwakeGameplayTags.h"
#include "Hellwake.h"

UHellwakeAttributeSet::UHellwakeAttributeSet()
{
	// Player defaults; enemies overwrite Health/MaxHealth from their
	// FHellwakeEnemyDefinition row on spawn (see HellwakeEnemyBase::BeginPlay).
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitHealthRegenRate(1.6f);
	InitWrath(100.f);
	InitMaxWrath(100.f);
	InitWrathRegenRate(4.5f);
	InitIncomingDamageMultiplier(1.f);
	InitDamage(0.f);
}

void UHellwakeAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHellwakeAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHellwakeAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHellwakeAttributeSet, HealthRegenRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHellwakeAttributeSet, Wrath, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHellwakeAttributeSet, MaxWrath, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHellwakeAttributeSet, WrathRegenRate, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHellwakeAttributeSet, IncomingDamageMultiplier, COND_None, REPNOTIFY_Always);
}

void UHellwakeAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetWrathAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxWrath());
	}
}

void UHellwakeAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	AActor* TargetActor = nullptr;
	AActor* InstigatorActor = nullptr;
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
	}
	if (Data.EffectSpec.GetEffectContext().GetInstigator())
	{
		InstigatorActor = Data.EffectSpec.GetEffectContext().GetInstigator();
	}

	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		// Prototype: `if (P.iframe > 0 || P.dead) return;` at the top of
		// damagePlayer() — i-frames (Dodge) and death both fully absorb
		// incoming damage, checked before the Bulwark multiplier applies.
		const UAbilitySystemComponent* TargetASCForGate = Data.Target.AbilityActorInfo.IsValid() ? Data.Target.AbilityActorInfo->AbilitySystemComponent.Get() : nullptr;
		const bool bAbsorbed = TargetASCForGate &&
			(TargetASCForGate->HasMatchingGameplayTag(HellwakeTags::State_IFrame) ||
			 TargetASCForGate->HasMatchingGameplayTag(HellwakeTags::State_Dead));
		if (bAbsorbed)
		{
			SetDamage(0.f);
			return;
		}

		// Bulwark (F / Ashen Bulwark) and any other "incoming damage
		// multiplier" effect scales the raw Damage meta-attribute here,
		// mirroring `amount * (P.bulwark > 0 ? 0.45 : 1)` in damagePlayer().
		const float RawDamage = GetDamage();
		const float ScaledDamage = FMath::Max(0.f, RawDamage * GetIncomingDamageMultiplier());
		SetDamage(0.f);

		if (ScaledDamage > 0.f)
		{
			const float NewHealth = FMath::Clamp(GetHealth() - ScaledDamage, 0.f, GetMaxHealth());
			SetHealth(NewHealth);
			LastDamageInstigator = InstigatorActor;

			UE_LOG(LogHellwakeCombat, Verbose, TEXT("%s took %.1f damage (raw %.1f) from %s — HP now %.1f/%.1f"),
				TargetActor ? *TargetActor->GetName() : TEXT("?"), ScaledDamage, RawDamage,
				InstigatorActor ? *InstigatorActor->GetName() : TEXT("?"), NewHealth, GetMaxHealth());

			if (TargetActor)
			{
				FGameplayEventData EventData;
				EventData.EventTag = HellwakeTags::Event_Damage_Taken;
				EventData.Instigator = InstigatorActor;
				EventData.Target = TargetActor;
				EventData.EventMagnitude = ScaledDamage;
				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, HellwakeTags::Event_Damage_Taken, EventData);
			}

			if (NewHealth <= 0.f && TargetActor)
			{
				FGameplayEventData DeathEvent;
				DeathEvent.EventTag = HellwakeTags::Event_Death;
				DeathEvent.Instigator = InstigatorActor;
				DeathEvent.Target = TargetActor;
				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, HellwakeTags::Event_Death, DeathEvent);
			}
		}
	}
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetWrathAttribute())
	{
		SetWrath(FMath::Clamp(GetWrath(), 0.f, GetMaxWrath()));
	}
}

void UHellwakeAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHellwakeAttributeSet, Health, OldValue);
}
void UHellwakeAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHellwakeAttributeSet, MaxHealth, OldValue);
}
void UHellwakeAttributeSet::OnRep_HealthRegenRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHellwakeAttributeSet, HealthRegenRate, OldValue);
}
void UHellwakeAttributeSet::OnRep_Wrath(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHellwakeAttributeSet, Wrath, OldValue);
}
void UHellwakeAttributeSet::OnRep_MaxWrath(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHellwakeAttributeSet, MaxWrath, OldValue);
}
void UHellwakeAttributeSet::OnRep_WrathRegenRate(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHellwakeAttributeSet, WrathRegenRate, OldValue);
}
void UHellwakeAttributeSet::OnRep_IncomingDamageMultiplier(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHellwakeAttributeSet, IncomingDamageMultiplier, OldValue);
}
