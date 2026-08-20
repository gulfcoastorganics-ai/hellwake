#include "Abilities/HellwakeAbility_Dodge.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HellwakeGameplayTags.h"

UHellwakeAbility_Dodge::UHellwakeAbility_Dodge()
{
	AbilityTags.AddTag(HellwakeTags::Ability_Dodge);
}

void UHellwakeAbility_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ACharacter* AvatarCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!AvatarCharacter || !ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Prototype: `moveDir()` from current WASD state, falling back to
	// facing if the player is standing still.
	FVector Direction = AvatarCharacter->GetLastMovementInputVector();
	if (Direction.IsNearlyZero())
	{
		Direction = AvatarCharacter->GetActorForwardVector();
	}
	Direction.Z = 0.f;
	Direction.Normalize();

	if (UCharacterMovementComponent* Movement = AvatarCharacter->GetCharacterMovement())
	{
		Movement->Velocity = Direction * ImpulseCmPerSec;
	}

	if (IFrameEffectClass)
	{
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddSourceObject(this);
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(IFrameEffectClass, GetAbilityLevel(), Context);
		if (Spec.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
		}
	}

	// TODO(VFX): burst(hero.position, 0.4, hero.position, 0xaab4c4, 0.5, 10)
	// -> spawn Niagara system NS_Dodge_Dust at AvatarCharacter's feet here.

	GetWorld()->GetTimerManager().SetTimer(EndHandle, this, &UHellwakeAbility_Dodge::FinishDodge, IFrameDuration, false);
}

void UHellwakeAbility_Dodge::FinishDodge()
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}
