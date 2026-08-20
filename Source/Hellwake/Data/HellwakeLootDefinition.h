// DataTable row for loot rarities. Mirrors the `RARITY` table in
// hellwake-game.js. Row name = rarity id ("common"/"magic"/"rare"/"legendary").
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "HellwakeLootDefinition.generated.h"

USTRUCT(BlueprintType)
struct HELLWAKE_API FHellwakeLootDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	FText SubLabel;

	// Rarity accent color — drives the beacon light/beam, the minimap blip,
	// and the HUD pickup toast. Prototype: RARITY[x].c.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	FLinearColor Color = FLinearColor::White;

	// Roll thresholds against a single FMath::FRand() when a non-elite enemy
	// drops loot: roll > RareThreshold -> rare, > MagicThreshold -> magic,
	// else common. Prototype: 0.93 / 0.6. Not used for the guaranteed
	// Legendary Gravewarden drop.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	float RollThreshold = 0.f;
};
