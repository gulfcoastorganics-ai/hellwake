// Base class for Ashbound Reaver, Cinder Wraith, and Pyre Acolyte. Owns its
// own ASC (each enemy is a self-contained ASC owner — simplest correct GAS
// setup for AI-controlled minions; see project-bible/unreal-implementation.md
// if you want to switch to a shared pool later for perf).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "Data/HellwakeEnemyDefinition.h"
#include "HellwakeEnemyBase.generated.h"

class UAbilitySystemComponent;
class UHellwakeAttributeSet;
class UHellwakeVitalityComponent;
class UHellwakeLootDropComponent;
class UGameplayEffect;
class UDataTable;
struct FGameplayEventData;

UCLASS()
class HELLWAKE_API AHellwakeEnemyBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AHellwakeEnemyBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, Category = "Hellwake|Enemy")
	void InitializeFromDefinition(const FHellwakeEnemyDefinition& Definition, FName RowName);

	UFUNCTION(BlueprintPure, Category = "Hellwake|Enemy")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, Category = "Hellwake|Enemy")
	float GetEngagementRingCm() const { return EngagementRingCm; }

	UPROPERTY(BlueprintReadOnly, Category = "Hellwake|Enemy")
	FGameplayTag RoleTag;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Abilities")
	TObjectPtr<UHellwakeAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Combat")
	TObjectPtr<UHellwakeVitalityComponent> VitalityComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Loot")
	TObjectPtr<UHellwakeLootDropComponent> LootDropComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Data")
	FName EnemyDefinitionRowName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Data")
	TObjectPtr<UDataTable> EnemyDefinitionTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(BlueprintReadOnly, Category = "Hellwake|Enemy")
	float MoveSpeedCm = 440.f;
	UPROPERTY(BlueprintReadOnly, Category = "Hellwake|Enemy")
	float AttackDamage = 10.f;
	UPROPERTY(BlueprintReadOnly, Category = "Hellwake|Enemy")
	float AttackRangeCm = 290.f;
	UPROPERTY(BlueprintReadOnly, Category = "Hellwake|Enemy")
	float AttackCooldownSeconds = 1.5f;
	UPROPERTY(BlueprintReadOnly, Category = "Hellwake|Enemy")
	float EngagementRingCm = 300.f;
	UPROPERTY(BlueprintReadOnly, Category = "Hellwake|Enemy")
	float LootDropChance = 0.35f;
	UPROPERTY(BlueprintReadOnly, Category = "Hellwake|Enemy")
	float XPReward = 40.f;
	UPROPERTY(BlueprintReadOnly, Category = "Hellwake|Enemy")
	bool bIsElite = false;

	UPROPERTY()
	float RingSlotRadians = 0.f;

	UPROPERTY()
	float AttackCooldownRemaining = 0.f;

	UPROPERTY()
	float StaggerRemaining = 0.f;

	UPROPERTY()
	bool bIsDead = false;

	UPROPERTY()
	float AnimTime = 0.f;

	UFUNCTION(BlueprintCallable, Category = "Hellwake|Combat")
	void DealDamageTo(AActor* Target, float Damage, bool bCrit = false);

	virtual void TryAttack(float DeltaSeconds, AActor* Target);
	AActor* FindPrimaryTarget() const;

	void OnDeathEvent(const FGameplayEventData* Payload);
	virtual void HandleDeath();
	virtual void UpdateAI(float DeltaSeconds, AActor* Target);
	virtual void UpdateDeathTick(float DeltaSeconds);

	UPROPERTY()
	float DespawnTimeRemaining = 0.9f;
};
