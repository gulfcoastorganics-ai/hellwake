// C++ base for the UMG HUD Blueprint widget (WBP_HellwakeHUD). Field-for-
// field mirror of the .dc.html HUD template's {{ }} interpolations /
// renderVals() output — see project-bible/hud.md for the exact mapping
// table.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/HellwakeEncounterStage.h"
#include "HellwakeHUDWidget.generated.h"

class APlayerController;
class UDataTable;
struct FGameplayEventData;

UCLASS(Abstract)
class HELLWAKE_API UHellwakeHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Hellwake|HUD")
	void BindToPlayerController(APlayerController* PC);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|HUD")
	TObjectPtr<UDataTable> LootDefinitionTable;

	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnHealthChanged(float Health, float MaxHealth);
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnWrathChanged(float Wrath, float MaxWrath);
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnLevelChanged(int32 Level);
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnXPChanged(float XPPercent);
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnStatusPipsChanged(const TArray<FString>& Labels, const TArray<FLinearColor>& Colors, const TArray<float>& RemainingSeconds);

	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnAbilityCooldownsChanged(float LMBPercent, float QPercent, float FPercent, float EPercent, float RPercent);

	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnBossVisibilityChanged(bool bVisible);
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnBossHealthChanged(float HealthPercent);
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnBossAffixesChanged(const FString& AffixesLine);

	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnObjectiveChanged(const FText& ObjectiveText, int32 AliveHostiles, int32 StageIndex, int32 StageTotal);
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnMinimapBlipsChanged(const TArray<FVector2D>& EnemyBlipOffsets, const TArray<bool>& bEliteFlags, const TArray<FVector2D>& LootBlipOffsets);

	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnBannerTextChanged(const FText& BannerText);
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnCinematicChanged(bool bCinematicActive);
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnStageLabelChanged(const FText& StageLabel);

	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnLootToast(const FText& ItemName, const FText& SubLabel, FLinearColor RarityColor);

private:
	UFUNCTION()
	void HandleHealthChanged(float NewValue, float MaxValue, float Delta);
	UFUNCTION()
	void HandleWrathChanged(float NewValue, float MaxValue, float Delta);
	UFUNCTION()
	void HandleStageChanged(EHellwakeEncounterStage NewStage);
	UFUNCTION()
	void HandleBannerRequested(const FText& BannerText);

	void HandleLootPickedUp(const FGameplayEventData* Payload);
};
