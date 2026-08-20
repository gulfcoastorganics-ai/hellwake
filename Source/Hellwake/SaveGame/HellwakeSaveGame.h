// The prototype has NO save/checkpoint system: death is a 1.8s respawn to
// a single fixed spawn point with full HP/Wrath restore and zero loss of
// encounter progress (enemies already killed stay dead only because the
// whole page hasn't reloaded — there is no persistence layer at all). This
// class is therefore forward-looking, not a port of existing behavior —
// added because a real production slice needs *some* answer for "what
// happens if the player quits mid-encounter," which the prototype never
// had to answer. Keep it minimal: enough to resume at the last completed
// stage, matching the spirit of "one 5-10 minute vertical slice" rather
// than building a full save system prematurely.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Data/HellwakeEncounterStage.h"
#include "HellwakeSaveGame.generated.h"

UCLASS()
class HELLWAKE_API UHellwakeSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Save")
	FString SlotName = TEXT("HellwakeSlot0");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Save")
	uint32 UserIndex = 0;

	// Highest stage reached — a fresh session starts the encounter fresh
	// (per the prototype's actual behavior) but this lets a title screen
	// show "Continue from: Gravewarden" for UX purposes.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hellwake|Save")
	EHellwakeEncounterStage HighestStageReached = EHellwakeEncounterStage::Enter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hellwake|Save")
	bool bLegendaryLootClaimed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hellwake|Save")
	float TotalPlaytimeSeconds = 0.f;
};
