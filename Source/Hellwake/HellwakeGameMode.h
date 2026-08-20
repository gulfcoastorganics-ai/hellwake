// Vertical-slice GameMode: starts the encounter subsystem, ticks it with
// the player's location, and spawns wave enemies when a stage's row lists
// any (see DT_EncounterStages' SpawnRowNames/SpawnLocations). This is the
// "gameplay framework" glue the prototype didn't need (it was one JS file
// with a single tick() function) — split out here so encounter pacing
// (UHellwakeEncounterSubsystem), enemy behavior (AHellwakeEnemyBase), and
// spawn orchestration (this class) stay independently testable.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Data/HellwakeEncounterStage.h"
#include "HellwakeGameMode.generated.h"

class UHellwakeEncounterSubsystem;
class UDataTable;
class AHellwakeEnemyBase;

UCLASS()
class HELLWAKE_API AHellwakeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHellwakeGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Data")
	TObjectPtr<UDataTable> EnemyDefinitionTable;

	UFUNCTION()
	void HandleStageChanged(EHellwakeEncounterStage NewStage);

private:
	UPROPERTY()
	TObjectPtr<UHellwakeEncounterSubsystem> Encounter;

	void SpawnEnemiesForStage(EHellwakeEncounterStage Stage);
};
