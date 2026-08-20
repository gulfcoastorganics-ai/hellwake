// Reference/balance-audit table only — NOT read at runtime. The actual
// authoritative tunables live as EditDefaultsOnly properties on each
// UHellwakeGameplayAbility subclass's Blueprint child (that's what
// CommitAbility/CanActivateAbility actually check). This table exists so
// design can see cost/cooldown/damage for all seven actions side by side
// without opening seven Blueprints — keep it in sync by hand, or promote it
// to the real source of truth later by having each ability read its row
// via RowName in InitializeAbility() if design wants that instead.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "HellwakeAbilityDefinition.generated.h"

USTRUCT(BlueprintType)
struct HELLWAKE_API FHellwakeAbilityDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	FGameplayTag AbilityTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	float WrathCost = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	float CooldownSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	float MinDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	float MaxDamage = 0.f;

	// HUD status pip text, e.g. "EMBER", "FORT". Empty if the ability has
	// no cosmetic/mechanical status effect.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	FText StatusPipLabel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake")
	float StatusDurationSeconds = 0.f;
};
