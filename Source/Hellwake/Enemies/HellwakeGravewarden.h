// The Gravewarden of the Ninth Seal — elite boss. Direct port of wardenAI()
// from hellwake-game.js: 3 HP-gated phases, weighted-random telegraphed
// attacks, Soulrend Aura DoT from phase 2, and add-spawns on each phase
// transition. See docs/hellwake-behavioral-spec.md §3 "Gravewarden phases"
// for the exact thresholds/weights this must match.
#pragma once

#include "CoreMinimal.h"
#include "Enemies/HellwakeEnemyBase.h"
#include "HellwakeGravewarden.generated.h"

UENUM(BlueprintType)
enum class EHellwakeGravewardenPhase : uint8
{
	Phase1 UMETA(DisplayName = "Phase 1 (>66% HP)"),
	Phase2 UMETA(DisplayName = "Phase 2 (33-66% HP, Soulrend Aura)"),
	Phase3 UMETA(DisplayName = "Phase 3 (<=33% HP, Corrupted Flame)")
};

UCLASS()
class HELLWAKE_API AHellwakeGravewarden : public AHellwakeEnemyBase
{
	GENERATED_BODY()

public:
	AHellwakeGravewarden();

	UFUNCTION(BlueprintPure, Category = "Hellwake|Gravewarden")
	EHellwakeGravewardenPhase GetPhase() const { return CurrentPhase; }

protected:
	virtual void UpdateAI(float DeltaSeconds, AActor* Target) override;
	virtual void HandleDeath() override;

	// Phase thresholds as HP fraction. Prototype: >0.66 P1, >0.33 P2, else P3.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden")
	float Phase2ThresholdPct = 0.66f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden")
	float Phase3ThresholdPct = 0.33f;

	// Soulrend Aura (phase 2+): 6 dmg/s within 9u (900cm) — implemented as
	// a per-tick probability matching the prototype's
	// `if (dist<9 && Math.random()<dt*1.6) damagePlayer(6)` rather than a
	// true DoT GameplayEffect, to stay a 1:1 port; a production version
	// should replace this with GE_Hellwake_SoulrendAura (periodic, applied/
	// removed on phase enter/exit) instead of the random-roll approach.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden")
	float SoulrendAuraRadiusCm = 900.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden")
	float SoulrendAuraDamagePerTick = 6.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden")
	float SoulrendAuraProcChancePerSecond = 1.6f;

	// Attack weights (sum need not be 1; treated as relative). Prototype:
	// 45% sweep, 30% slam, 25% area denial.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Attacks")
	float SweepWeight = 0.45f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Attacks")
	float SlamWeight = 0.30f;
	// Remaining weight goes to Area Denial.

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Attacks")
	float SweepRangeCm = 800.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Attacks")
	float SweepRadiusCm = 650.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Attacks")
	float SweepDamage = 24.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Attacks")
	float SweepTelegraphSeconds = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Attacks")
	float SlamRadiusCm = 550.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Attacks")
	float SlamDamage = 32.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Attacks")
	float SlamTelegraphSeconds = 1.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Attacks")
	float PillarRadiusCm = 320.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Attacks")
	float PillarDamage = 18.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Attacks")
	float PillarMinRangeCm = 500.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Attacks")
	float PillarMaxRangeCm = 1900.f;

	// Adds spawned on phase transition. AHellwakeEnemyBase subclasses +
	// EnemyDefinitionTable row names, resolved via SpawnActorHelper in the
	// .cpp. Wired in-editor to BP_AshboundReaver / BP_CinderWraith.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Adds")
	TSubclassOf<AHellwakeEnemyBase> ReaverClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Adds")
	TSubclassOf<AHellwakeEnemyBase> WraithClass;

	// Legendary loot definition row, spawned guaranteed on death.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Loot")
	TSubclassOf<class AHellwakeLootPickup> LegendaryLootPickupClass;

private:
	EHellwakeGravewardenPhase CurrentPhase = EHellwakeGravewardenPhase::Phase1;
	float AttackCastTimeRemaining = 0.f;

	void EnterPhase(EHellwakeGravewardenPhase NewPhase);
	void ChooseAndBeginAttack(AActor* Target);
	void SpawnAdds(TSubclassOf<AHellwakeEnemyBase> EnemyClass, int32 Count, float SideOffsetCm);
};
