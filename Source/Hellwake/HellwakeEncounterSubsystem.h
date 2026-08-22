// Owns the 8-stage encounter state machine. Direct port of the STAGES
// array + setStage()/the stage-transition checks inside tick() from
// hellwake-game.js.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/HellwakeEncounterStage.h"
#include "HellwakeEncounterSubsystem.generated.h"

class UDataTable;
class AHellwakeEnemyBase;
class AHellwakeGravewarden;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHellwakeStageChanged, EHellwakeEncounterStage, NewStage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHellwakeBannerRequested, const FText&, BannerText);

UCLASS()
class HELLWAKE_API UHellwakeEncounterSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	UFUNCTION(BlueprintCallable, Category = "Hellwake|Encounter")
	void BeginEncounter();

	UFUNCTION(BlueprintCallable, Category = "Hellwake|Encounter")
	void Tick(float DeltaSeconds, const FVector& PlayerLocation);

	UFUNCTION(BlueprintPure, Category = "Hellwake|Encounter")
	EHellwakeEncounterStage GetCurrentStage() const { return CurrentStage; }

	UFUNCTION(BlueprintPure, Category = "Hellwake|Encounter")
	FText GetObjectiveText() const;

	UFUNCTION(BlueprintPure, Category = "Hellwake|Encounter")
	FText GetStageLabel() const;

	UFUNCTION(BlueprintPure, Category = "Hellwake|Encounter")
	int32 GetAliveHostileCount() const;

	UFUNCTION(BlueprintPure, Category = "Hellwake|Encounter")
	AHellwakeGravewarden* GetGravewarden() const { return TrackedGravewarden; }

	UFUNCTION(BlueprintCallable, Category = "Hellwake|Encounter")
	void NotifyLegendaryLootTaken() { bLegendaryLootTaken = true; }

	UPROPERTY(BlueprintAssignable, Category = "Hellwake|Encounter")
	FHellwakeStageChanged OnStageChanged;

	UPROPERTY(BlueprintAssignable, Category = "Hellwake|Encounter")
	FHellwakeBannerRequested OnBannerRequested;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Encounter")
	TObjectPtr<UDataTable> EncounterStageTable;

	UFUNCTION(BlueprintCallable, Category = "Hellwake|Encounter")
	void RegisterActiveWaveEnemies(const TArray<AHellwakeEnemyBase*>& Enemies);

	UFUNCTION(BlueprintCallable, Category = "Hellwake|Encounter")
	void RegisterGravewarden(AHellwakeGravewarden* Warden) { TrackedGravewarden = Warden; }

private:
	EHellwakeEncounterStage CurrentStage = EHellwakeEncounterStage::Enter;
	float StageTimer = 0.f;
	bool bLegendaryLootTaken = false;

	UPROPERTY()
	TArray<TObjectPtr<AHellwakeEnemyBase>> TrackedWaveEnemies;

	UPROPERTY()
	TObjectPtr<AHellwakeGravewarden> TrackedGravewarden;

	void SetStage(EHellwakeEncounterStage NewStage);
	const FHellwakeEncounterStageDefinition* GetRow(EHellwakeEncounterStage Stage) const;
	static FName StageToRowName(EHellwakeEncounterStage Stage);
};
