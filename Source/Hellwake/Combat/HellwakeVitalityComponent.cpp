#include "Combat/HellwakeVitalityComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Attributes/HellwakeAttributeSet.h"

UHellwakeVitalityComponent::UHellwakeVitalityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHellwakeVitalityComponent::BeginPlay()
{
	Super::BeginPlay();

	IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(GetOwner());
	CachedASC = ASI ? ASI->GetAbilitySystemComponent() : nullptr;
	if (!CachedASC)
	{
		return;
	}

	CachedASC->GetGameplayAttributeValueChangeDelegate(UHellwakeAttributeSet::GetHealthAttribute())
		.AddUObject(this, &UHellwakeVitalityComponent::HandleHealthChange);
	CachedASC->GetGameplayAttributeValueChangeDelegate(UHellwakeAttributeSet::GetWrathAttribute())
		.AddUObject(this, &UHellwakeVitalityComponent::HandleWrathChange);
}

float UHellwakeVitalityComponent::GetHealthPercent() const
{
	if (!CachedASC) return 1.f;
	const UHellwakeAttributeSet* Attr = CachedASC->GetSet<UHellwakeAttributeSet>();
	return Attr && Attr->GetMaxHealth() > 0.f ? Attr->GetHealth() / Attr->GetMaxHealth() : 0.f;
}

float UHellwakeVitalityComponent::GetWrathPercent() const
{
	if (!CachedASC) return 1.f;
	const UHellwakeAttributeSet* Attr = CachedASC->GetSet<UHellwakeAttributeSet>();
	return Attr && Attr->GetMaxWrath() > 0.f ? Attr->GetWrath() / Attr->GetMaxWrath() : 0.f;
}

void UHellwakeVitalityComponent::HandleHealthChange(const FOnAttributeChangeData& Data)
{
	const UHellwakeAttributeSet* Attr = CachedASC ? CachedASC->GetSet<UHellwakeAttributeSet>() : nullptr;
	const float Max = Attr ? Attr->GetMaxHealth() : 100.f;
	OnHealthChanged.Broadcast(Data.NewValue, Max, Data.NewValue - Data.OldValue);
}

void UHellwakeVitalityComponent::HandleWrathChange(const FOnAttributeChangeData& Data)
{
	const UHellwakeAttributeSet* Attr = CachedASC ? CachedASC->GetSet<UHellwakeAttributeSet>() : nullptr;
	const float Max = Attr ? Attr->GetMaxWrath() : 100.f;
	OnWrathChanged.Broadcast(Data.NewValue, Max, Data.NewValue - Data.OldValue);
}
