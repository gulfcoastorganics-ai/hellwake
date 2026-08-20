#include "Loot/HellwakeLootDropComponent.h"
#include "Loot/HellwakeLootPickup.h"
#include "Data/HellwakeLootDefinition.h"
#include "Engine/DataTable.h"

UHellwakeLootDropComponent::UHellwakeLootDropComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHellwakeLootDropComponent::RollAndMaybeDrop(float DropChance, const FVector& Location)
{
	if (!LootPickupClass || !GetWorld() || FMath::FRand() >= DropChance)
	{
		return;
	}

	// Prototype rolls a single second FRand() and thresholds it directly
	// (roll > 0.93 rare, > 0.6 magic, else common) rather than a weighted
	// table walk. Reproduced by picking the highest-threshold row the roll
	// clears, walking RollableRarityRows in the order provided (expected
	// common -> magic -> rare, ascending RollThreshold).
	const float RarityRoll = FMath::FRand();
	FName ChosenRow = RollableRarityRows.Num() > 0 ? RollableRarityRows[0] : NAME_None;
	if (LootDefinitionTable)
	{
		for (const FName& RowName : RollableRarityRows)
		{
			const FHellwakeLootDefinition* Row = LootDefinitionTable->FindRow<FHellwakeLootDefinition>(RowName, TEXT("HellwakeLootDropComponent::RollAndMaybeDrop"));
			if (Row && RarityRoll > Row->RollThreshold)
			{
				ChosenRow = RowName;
			}
		}
	}

	if (ChosenRow.IsNone())
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AHellwakeLootPickup* Pickup = GetWorld()->SpawnActor<AHellwakeLootPickup>(LootPickupClass, Location, FRotator::ZeroRotator, Params);
	if (Pickup)
	{
		Pickup->RarityRowName = ChosenRow;
		Pickup->LootDefinitionTable = LootDefinitionTable;
	}
}
