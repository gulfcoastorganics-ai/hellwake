// DataTable row for enemy archetypes. Mirrors the `ROLE` table in
// hellwake-game.js exactly (see docs/hellwake-behavioral-spec.md §3).
// Import Content/Data/EnemyDefinitions.csv as DataTable
// (RowStruct = FHellwakeEnemyDefinition) in-editor to get DT_EnemyDefinitions.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "HellwakeEnemyDefinition.generated.h"

USTRUCT(BlueprintType)
struct HELLWAKE_API FHellwakeEnemyDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	FText DisplayName;

	// Enemy_Role_* tag — used by the State Tree AI to select behavior tasks.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	FGameplayTag RoleTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	float MaxHealth = 100.f;

	// cm/s. Prototype speed is in u/s (1u = 100cm); pre-converted here.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	float MoveSpeedCm = 440.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	float AttackDamage = 10.f;

	// cm. Prototype `range`.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	float AttackRangeCm = 290.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	float AttackCooldownSeconds = 1.5f;

	// cm. The formation radius this enemy tries to hold around the player
	// (`def.ring` in the prototype) — the "surround, don't stack" behavior.
	// For the Pyre Acolyte this is also effectively its preferred combat
	// range, which is why it never closes distance; see
	// docs/hellwake-behavioral-spec.md §2.2 before reusing this value
	// verbatim for the AI's "give up and approach" fallback distance.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	float EngagementRingCm = 300.f;

	// 0-1 chance to drop loot on death (non-elite only).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	float LootDropChance = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	float XPReward = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	bool bIsElite = false;

	// AHellwakeEnemyBase subclass (or AHellwakeGravewarden for the elite row).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	TSoftClassPtr<AActor> PawnClass;
};
