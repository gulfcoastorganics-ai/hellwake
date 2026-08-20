// Base class for Ashbound Reaver, Cinder Wraith, and Pyre Acolyte. Owns its
// own ASC (each enemy is a self-contained ASC owner — simplest correct GAS
// setup for AI-controlled minions; see project-bible/unreal-implementation.md
// if you want to switch to a shared pool later for perf).
//
// AI: this class's Tick() is a direct, verified port of enemyAI(e, dt) from
// hellwake-game.js — ring-hold positioning around the player, melee/ranged
// attack selection by RoleTag, telegraphed ranged bolts for the Acolyte.
// It is the *reference behavior*, written in C++ so it is runnable and
// testable immediately; project-bible/enemies.md documents how to lift each
// piece into a State Tree (NavMesh MoveTo task for the ring-hold, a
// Sequence for the telegraph-then-fire ranged attack, etc.) as the intended
// production AI authoring path once the Editor is available. Do not delete
// this Tick fallback until the State Tree replacement is verified to
// reproduce it — it's the only currently-testable copy of this behavior.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "HellwakeEnemyBase.generated.h"

class UAbilitySystemComponent;
class UHellwakeAttributeSet;
class UHellwakeVitalityComponent;
class UGameplayEffect;
struct FHellwakeEnemyDefinition;

UCLASS()
class HELLWAKE_API AHellwakeEnemyBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AHellwakeEnemyBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Applies a DataTable row (see Data/HellwakeEnemyDefinition.h) to this
	// instance's attributes/tunables. Call right after SpawnActor, before
	// BeginPlay does its own init (or re-call to hot-swap for testing).
	UFUNCTION(BlueprintCallable, Category = "Hellwake|Enemy")
	void InitializeFromDefinition(const FHellwakeEnemyDefinition& Definition, FName RowName);

	UFUNCTION(BlueprintPure, Category = "Hellwake|Enemy")
	bool IsDead() const { return bIsDead; }

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
	TObjectPtr<class UHellwakeLootDropComponent> LootDropComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Data")
	FName EnemyDefinitionRowName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Data")
	TObjectPtr<class UDataTable> EnemyDefinitionTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Combat")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// --- Ported tunables (see FHellwakeEnemyDefinition for authoritative field docs) ---
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

	// Random angular slot on the engagement ring, radians. Prototype:
	// `e.slot = Math.random() * Math.PI * 2`; Cinder Wraiths additionally
	// orbit (`slot + anim*0.5`).
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

	// Deals damage via DamageEffectClass to the target actor's ASC.
	UFUNCTION(BlueprintCallable, Category = "Hellwake|Combat")
	void DealDamageTo(AActor* Target, float Damage, bool bCrit = false);

	// Tries a melee/ranged attack against Target if in range and off
	// cooldown. Called from Tick(); split out so AHellwakeGravewarden's
	// override can reuse the cooldown/range plumbing for its own attacks.
	virtual void TryAttack(float DeltaSeconds, AActor* Target);

	// Finds the current combat target (the player pawn — this vertical
	// slice is single-target/single-player by design, matching the
	// prototype's single `hero` reference).
	AActor* FindPrimaryTarget() const;

	void OnDeathEvent(const FGameplayEventData* Payload);
	virtual void HandleDeath();

	// Per-frame AI while alive. Base: ring-hold positioning + TryAttack.
	// AHellwakeGravewarden overrides this entirely with its 3-phase logic.
	virtual void UpdateAI(float DeltaSeconds, AActor* Target);

	// Post-death sink/tumble before despawn. Prototype:
	// `e.obj.position.y -= dt*1.2; e.obj.rotation.z += dt*1.1;` then removed
	// after 0.9s (2.6s for elites).
	virtual void UpdateDeathTick(float DeltaSeconds);

	UPROPERTY()
	float DespawnTimeRemaining = 0.9f;
};
