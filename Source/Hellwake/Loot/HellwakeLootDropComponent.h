// Attach to non-elite enemies. Rolls LootDropChance on death and, if it
// hits, rolls rarity against DT_LootDefinitions' RollThreshold column and
// spawns an AHellwakeLootPickup. Prototype:
//   if (!e.def.elite && Math.random() < e.def.drop) {
//     const roll = Math.random();
//     dropLoot(x, z, roll > 0.93 ? 'rare' : roll > 0.6 ? 'magic' : 'common');
//   }
// The Gravewarden does not use this component — its guaranteed Legendary
// drop is handled directly in AHellwakeGravewarden::HandleDeath.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HellwakeLootDropComponent.generated.h"

class AHellwakeLootPickup;
class UDataTable;

UCLASS(ClassGroup = (Hellwake), meta = (BlueprintSpawnableComponent))
class HELLWAKE_API UHellwakeLootDropComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHellwakeLootDropComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Loot")
	TSubclassOf<AHellwakeLootPickup> LootPickupClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Loot")
	TObjectPtr<UDataTable> LootDefinitionTable;

	// Rows to roll against, ordered common -> legendary. Prototype never
	// drops "legendary" from a random roll (only the guaranteed Gravewarden
	// reward), so this defaults to the first three rarities only.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Loot")
	TArray<FName> RollableRarityRows = { TEXT("common"), TEXT("magic"), TEXT("rare") };

	// Called from AHellwakeEnemyBase::HandleDeath. DropChance is the
	// enemy's FHellwakeEnemyDefinition::LootDropChance.
	UFUNCTION(BlueprintCallable, Category = "Hellwake|Loot")
	void RollAndMaybeDrop(float DropChance, const FVector& Location);
};
