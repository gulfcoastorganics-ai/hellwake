// Core GAS AttributeSet for Hellwake — shared by the player and all enemies
// (including the Gravewarden). Mirrors the prototype's P.hp/P.hpMax/P.wrath/
// P.wrathMax and the enemy e.hp/e.hpMax fields (hellwake-game.js).
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "HellwakeAttributeSet.generated.h"

// Boilerplate accessor macro from the GAS samples — generates Get/Set/Init
// helpers per attribute (GetHealthAttribute, GetHealth, SetHealth, ...).
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Shared between PlayerState (Kaervoss) and enemy pawns. Enemies only ever
 * use Health/MaxHealth; Wrath/MaxWrath are player-only but kept on the same
 * set so a single AttributeSet class covers every ASC-owning actor, matching
 * how the prototype keeps player and enemy state in parallel shapes (P.* vs
 * e.hp/e.hpMax) without a shared base — here we actually share it.
 */
UCLASS()
class HELLWAKE_API UHellwakeAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UHellwakeAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// --- Vitality ---
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Hellwake|Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UHellwakeAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Hellwake|Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UHellwakeAttributeSet, MaxHealth)

	// Passive health regen, HP/s. Prototype: dt * 1.6 while alive and not full.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_HealthRegenRate, Category = "Hellwake|Attributes")
	FGameplayAttributeData HealthRegenRate;
	ATTRIBUTE_ACCESSORS(UHellwakeAttributeSet, HealthRegenRate)

	// --- Resource ("Wrath") — player only ---
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Wrath, Category = "Hellwake|Attributes")
	FGameplayAttributeData Wrath;
	ATTRIBUTE_ACCESSORS(UHellwakeAttributeSet, Wrath)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxWrath, Category = "Hellwake|Attributes")
	FGameplayAttributeData MaxWrath;
	ATTRIBUTE_ACCESSORS(UHellwakeAttributeSet, MaxWrath)

	// Passive resource regen, Wrath/s. Prototype: dt * 4.5.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WrathRegenRate, Category = "Hellwake|Attributes")
	FGameplayAttributeData WrathRegenRate;
	ATTRIBUTE_ACCESSORS(UHellwakeAttributeSet, WrathRegenRate)

	// --- Combat scalars ---
	// Multiplies incoming Damage before it reduces Health. Bulwark (F) sets
	// this to 0.45 for 10s, matching `P.bulwark > 0 ? 0.45 : 1` in the
	// prototype's damagePlayer().
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IncomingDamageMultiplier, Category = "Hellwake|Attributes")
	FGameplayAttributeData IncomingDamageMultiplier;
	ATTRIBUTE_ACCESSORS(UHellwakeAttributeSet, IncomingDamageMultiplier)

	// 0-100. Prototype: `P.xp = Math.min(100, P.xp + e.def.xp/12)` — a
	// simplified single-bar XP meter for the vertical slice (no leveling
	// logic; Level is a fixed cosmetic 42 in the prototype's own HUD).
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_XP, Category = "Hellwake|Attributes")
	FGameplayAttributeData XP;
	ATTRIBUTE_ACCESSORS(UHellwakeAttributeSet, XP)

	// Meta attribute: never persisted, only used as the execution target for
	// GameplayEffects that deal damage. PostGameplayEffectExecute converts
	// any change here into a Health reduction and resets it to 0.
	UPROPERTY(BlueprintReadOnly, Category = "Hellwake|Attributes")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UHellwakeAttributeSet, Damage)

protected:
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_HealthRegenRate(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_Wrath(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_MaxWrath(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_WrathRegenRate(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_IncomingDamageMultiplier(const FGameplayAttributeData& OldValue);
	UFUNCTION()
	virtual void OnRep_XP(const FGameplayAttributeData& OldValue);

private:
	// Cached from the Damage GameplayEffect's source/instigator so the death
	// event and combat log can report who/what killed this actor.
	TWeakObjectPtr<AActor> LastDamageInstigator;
};

#undef ATTRIBUTE_ACCESSORS
