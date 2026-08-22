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

	UFUNCTION(BlueprintPure, Category = "Hellwake|Gravewarden")
	int32 GetAliveAddCount() const;

protected:
	virtual void UpdateAI(float DeltaSeconds, AActor* Target) override;
	virtual void HandleDeath() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden")
	float Phase2ThresholdPct = 0.66f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden")
	float Phase3ThresholdPct = 0.33f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden")
	float SoulrendAuraRadiusCm = 900.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden")
	float SoulrendAuraDamagePerTick = 6.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden")
	float SoulrendAuraProcChancePerSecond = 1.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Attacks")
	float SweepWeight = 0.45f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Attacks")
	float SlamWeight = 0.30f;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Adds")
	TSubclassOf<AHellwakeEnemyBase> ReaverClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Adds")
	TSubclassOf<AHellwakeEnemyBase> WraithClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Gravewarden|Loot")
	TSubclassOf<class AHellwakeLootPickup> LegendaryLootPickupClass;

private:
	EHellwakeGravewardenPhase CurrentPhase = EHellwakeGravewardenPhase::Phase1;
	float AttackCastTimeRemaining = 0.f;

	UPROPERTY()
	TArray<TObjectPtr<AHellwakeEnemyBase>> SpawnedAdds;

	void EnterPhase(EHellwakeGravewardenPhase NewPhase);
	void ChooseAndBeginAttack(AActor* Target);
	void SpawnAdds(TSubclassOf<AHellwakeEnemyBase> EnemyClass, FName DefinitionRowName, int32 Count, float SideOffsetCm);
};
