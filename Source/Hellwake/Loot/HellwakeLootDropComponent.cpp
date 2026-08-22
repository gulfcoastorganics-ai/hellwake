#include "Loot/HellwakeLootDropComponent.h"
#include "Loot/HellwakeLootPickup.h"
#include "Data/HellwakeLootDefinition.h"
#include "Engine/DataTable.h"

UHellwakeLootDropComponent::UHellwakeLootDropComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	LootPickupClass = AHellwakeLootPickup::StaticClass();
}

void UHellwakeLootDropComponent::RollAndMaybeDrop(float DropChance, const FVector& Location)
{
	if (!LootPickupClass || !GetWorld() || FMath::FRand() >= DropChance)
	{
		return;
	}

	const float RarityRoll = FMath::FRand();
	FName ChosenRow = TEXT("common");
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
	else
	{
		// Exact prototype fallback when DT_LootDefinitions has not been
		// imported yet: >.93 rare, >.60 magic, otherwise common.
		ChosenRow = RarityRoll > 0.93f ? TEXT("rare") : (RarityRoll > 0.60f ? TEXT("magic") : TEXT("common"));
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AHellwakeLootPickup* Pickup = GetWorld()->SpawnActor<AHellwakeLootPickup>(LootPickupClass, Location + FVector(0.f, 0.f, 30.f), FRotator::ZeroRotator, Params);
	if (Pickup)
	{
		Pickup->ConfigurePickup(ChosenRow, LootDefinitionTable);
	}
}
