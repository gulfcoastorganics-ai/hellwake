// DataTable row for the 8-stage encounter flow. Mirrors the `STAGES` array
// in hellwake-game.js. Row name = stage id ("enter", "wave1", ...).
// Consumed by UHellwakeEncounterSubsystem, which owns the actual transition
// *logic* (position thresholds, alive-count checks) since those aren't
// pure data — see HellwakeEncounterSubsystem.h.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "HellwakeEncounterStage.generated.h"

UENUM(BlueprintType)
enum class EHellwakeEncounterStage : uint8
{
	Enter,
	Wave1,
	Advance,
	Wave2,
	Intro,
	Boss,
	Reward,
	Done
};

USTRUCT(BlueprintType)
struct HELLWAKE_API FHellwakeEncounterStageDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	EHellwakeEncounterStage StageId = EHellwakeEncounterStage::Enter;

	// Objective text shown in the top-right HUD panel while this stage is
	// active. Prototype: STAGES[i].text.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	FText ObjectiveText;

	// Bottom-right compact stage label. Prototype: stageNames map in
	// renderVals().
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	FText StageLabel;

	// Full-screen banner text shown for ~3s on entering this stage.
	// Prototype: banner(s.text.toUpperCase()) — reusing ObjectiveText
	// uppercased in the source; kept as a separate field here so design can
	// diverge (e.g. shorter banner copy) without touching the objective text.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	FText BannerText;

	// Enemy spawns fired when entering this stage: (RoleTag row name, spawn
	// point). Populated for Wave1/Wave2/Intro only — Enter/Advance/Boss/
	// Reward/Done have none. This is data-authored per the prototype's
	// hardcoded spawn(...) calls in setStage(), which do not scale/relocate
	// with level geometry — a real level should replace these with
	// designer-placed spawn markers instead of literal coordinates; kept
	// here as the ported baseline.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	TArray<FName> SpawnRowNames;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	TArray<FVector> SpawnLocations;
};
