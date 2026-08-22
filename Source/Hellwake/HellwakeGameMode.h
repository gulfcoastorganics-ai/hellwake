// Vertical-slice GameMode: owns encounter orchestration and an asset-free
// fallback arena so a clean native checkout is playable before any .umap,
// DataTable, Blueprint, or UMG assets have been authored in-editor.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Data/HellwakeEncounterStage.h"
#include "HellwakeGameMode.generated.h"

class UHellwakeEncounterSubsystem;
class UDataTable;
class UStaticMesh;

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

	// Keep enabled until L_Hellwake_VerticalSlice exists. It creates a floor,
	// boundary walls, and basic lighting using Engine content only.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Fallback")
	bool bBuildNativeFallbackArena = true;

	UPROPERTY()
	TObjectPtr<UStaticMesh> ArenaCubeMesh;

	UFUNCTION()
	void HandleStageChanged(EHellwakeEncounterStage NewStage);

private:
	UPROPERTY()
	TObjectPtr<UHellwakeEncounterSubsystem> Encounter;

	void BuildNativeFallbackArena();
	void SpawnEnemiesForStage(EHellwakeEncounterStage Stage);
};
