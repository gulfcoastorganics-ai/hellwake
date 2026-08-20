// Blueprint/UMG-facing wrapper over UHellwakeAttributeSet. GAS attribute
// change delegates (FOnGameplayAttributeValueChange) aren't
// BlueprintAssignable, so the HUD widget can't bind to them directly; this
// component listens on BeginPlay and re-broadcasts as dynamic multicast
// delegates the widget *can* bind to in the Designer. This is the
// "health/resource component" the prototype's P.hp/P.hpMax/P.wrath/
// P.wrathMax fields map onto, kept as a thin façade rather than the source
// of truth (the AttributeSet/ASC remain that) so GAS prediction/replication
// still works correctly.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HellwakeVitalityComponent.generated.h"

class UHellwakeAttributeSet;
class UAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FHellwakeVitalityChanged, float, NewValue, float, MaxValue, float, Delta);

UCLASS(ClassGroup = (Hellwake), meta = (BlueprintSpawnableComponent))
class HELLWAKE_API UHellwakeVitalityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHellwakeVitalityComponent();

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintAssignable, Category = "Hellwake|Vitality")
	FHellwakeVitalityChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Hellwake|Vitality")
	FHellwakeVitalityChanged OnWrathChanged;

	UFUNCTION(BlueprintPure, Category = "Hellwake|Vitality")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Hellwake|Vitality")
	float GetWrathPercent() const;

private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedASC;

	void HandleHealthChange(const struct FOnAttributeChangeData& Data);
	void HandleWrathChange(const struct FOnAttributeChangeData& Data);
};
