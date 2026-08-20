// Owns the 8-stage encounter state machine. Direct port of the STAGES
// array + setStage()/the stage-transition checks inside tick() from
// hellwake-game.js. Reads stage text/labels/spawns from
// DT_EncounterStages (Data/HellwakeEncounterStage.h); owns the *transition
// logic* itself since that's conditional code (position thresholds,
// alive-enemy counts), not pure data.
//
// This is a UWorldSubsystem (not an ActorComponent on GameMode) so both the
// HUD widget and any gameplay actor can query CurrentStage/GetObjectiveText
// without needing a GameMode reference — matches how the prototype's
// `stage()`/`STAGES[stageIdx]` were freely readable from the single render
// loop.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/HellwakeEncounterStage.h"
#include "HellwakeEncounterSubsystem.generated.h"

class UDataTable;
class AHellwakeEnemyBase;
class AHellwakeGravewarden;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHellwakeStageChanged, EHellwakeEncounterStage, NewStage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHellwakeBannerRequested, const FText&, BannerText);

UCLASS()
class HELLWAKE_API UHellwakeEncounterSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorld(UWorld* World) const override;

	// Called once by the GameMode/level BeginPlay after the player pawn
	// exists. Enters Stage::Enter.
	UFUNCTION(BlueprintCallable, Category = "Hellwake|Encounter")
	void BeginEncounter();

	// Drive the state machine — call every tick from the GameMode (or a
	// dedicated tickable actor). Threshold checks:
	//   enter -> wave1:   player.Y < 1200 (prototype hero.z < 12)
	//   wave1 -> advance: all wave1 spawns dead, +1s settle
	//   advance -> wave2: player.Y < -400 (prototype hero.z < -4)
	//   wave2 -> intro:   all wave2 spawns dead, +1s settle
	//   intro -> boss:    3.4s elapsed
	//   boss -> reward:   Gravewarden dead, +2.2s settle
	//   reward -> done:   legendary loot taken, +1s settle
	UFUNCTION(BlueprintCallable, Category = "Hellwake|Encounter")
	void Tick(float DeltaSeconds, const FVector& PlayerLocation);

	UFUNCTION(BlueprintPure, Category = "Hellwake|Encounter")
	EHellwakeEncounterStage GetCurrentStage() const { return CurrentStage; }

	UFUNCTION(BlueprintPure, Category = "Hellwake|Encounter")
	FText GetObjectiveText() const;

	UFUNCTION(BlueprintPure, Category = "Hellwake|Encounter")
	FText GetStageLabel() const;

	UFUNCTION(BlueprintPure, Category = "Hellwake|Encounter")
	int32 GetAliveHostileCount() const;

	UFUNCTION(BlueprintCallable, Category = "Hellwake|Encounter")
	void NotifyLegendaryLootTaken() { bLegendaryLootTaken = true; }

	UPROPERTY(BlueprintAssignable, Category = "Hellwake|Encounter")
	FHellwakeStageChanged OnStageChanged;

	UPROPERTY(BlueprintAssignable, Category = "Hellwake|Encounter")
	FHellwakeBannerRequested OnBannerRequested;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Encounter")
	TObjectPtr<UDataTable> EncounterStageTable;

	// Wired by the GameMode/level after spawning the wave1/wave2 enemies —
	// see project-bible/vertical-slice.md for why spawn coordination lives
	// outside this subsystem (spawn points should be designer-placed level
	// actors, not this subsystem's concern).
	UFUNCTION(BlueprintCallable, Category = "Hellwake|Encounter")
	void RegisterActiveWaveEnemies(const TArray<AHellwakeEnemyBase*>& Enemies);

	UFUNCTION(BlueprintCallable, Category = "Hellwake|Encounter")
	void RegisterGravewarden(AHellwakeGravewarden* Warden) { TrackedGravewarden = Warden; }

private:
	EHellwakeEncounterStage CurrentStage = EHellwakeEncounterStage::Enter;
	float StageTimer = 0.f;
	bool bLegendaryLootTaken = false;

	UPROPERTY()
	TArray<TObjectPtr<AHellwakeEnemyBase>> TrackedWaveEnemies;

	UPROPERTY()
	TObjectPtr<AHellwakeGravewarden> TrackedGravewarden;

	void SetStage(EHellwakeEncounterStage NewStage);
	const FHellwakeEncounterStageDefinition* GetRow(EHellwakeEncounterStage Stage) const;
	static FName StageToRowName(EHellwakeEncounterStage Stage);
};
