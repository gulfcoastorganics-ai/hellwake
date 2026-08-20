// C++ base for the UMG HUD Blueprint widget (WBP_HellwakeHUD). Field-for-
// field mirror of the .dc.html HUD template's {{ }} interpolations /
// renderVals() output — see project-bible/hud.md for the exact mapping
// table. This class does the *binding* (attribute delegates, encounter
// subsystem delegates, gameplay events) and exposes
// BlueprintImplementableEvents; the actual widget tree (health bar image,
// Cinzel/Barlow Condensed fonts, diegetic panel shapes) is authored in the
// UMG Designer against WBP_HellwakeHUD, which should inherit from this class.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/HellwakeEncounterStage.h"
#include "HellwakeHUDWidget.generated.h"

class APlayerController;
class UDataTable;

UCLASS(Abstract)
class HELLWAKE_API UHellwakeHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Called once from AHellwakePlayerController::BeginPlay after AddToViewport.
	UFUNCTION(BlueprintCallable, Category = "Hellwake|HUD")
	void BindToPlayerController(APlayerController* PC);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|HUD")
	TObjectPtr<UDataTable> LootDefinitionTable;

	// --- Player vitality (KAERVOSS panel, top-left) ---
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnHealthChanged(float Health, float MaxHealth);
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnWrathChanged(float Wrath, float MaxWrath);
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnLevelChanged(int32 Level);
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnXPChanged(float XPPercent);
	// short label ("EMBER"), color, remaining seconds — one call per active status.
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnStatusPipsChanged(const TArray<FString>& Labels, const TArray<FLinearColor>& Colors, const TArray<float>& RemainingSeconds);

	// --- Ability bar (bottom-center) ---
	// Percent 0-100 remaining on cooldown per slot (LMB, Q, F/W, E, R),
	// matching `cds[key] / CD[key] * 100` in renderVals().
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnAbilityCooldownsChanged(float LMBPercent, float QPercent, float FPercent, float EPercent, float RPercent);

	// --- Boss panel (top-center, GRAVEWARDEN / ELITE) ---
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnBossVisibilityChanged(bool bVisible);
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnBossHealthChanged(float HealthPercent);
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnBossAffixesChanged(const FString& AffixesLine);

	// --- Objective panel (top-right) + minimap ---
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnObjectiveChanged(const FText& ObjectiveText, int32 AliveHostiles, int32 StageIndex, int32 StageTotal);
	// Minimap blips: normalized [-1,1] offset from player, per prototype's
	// `(50 + b.x*50)%` conversion (here as raw -1..1 instead of 0..100%).
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnMinimapBlipsChanged(const TArray<FVector2D>& EnemyBlipOffsets, const TArray<bool>& bEliteFlags, const TArray<FVector2D>& LootBlipOffsets);

	// --- Center banner + cinematic letterbox ---
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnBannerTextChanged(const FText& BannerText);
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnCinematicChanged(bool bCinematicActive);
	UFUNCTION(BlueprintImplementableEvent, Category = "Hellwake|HUD")
	void OnStageLabelChanged(const FText& StageLabel);

	// --- Loot pickup toast ---
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
