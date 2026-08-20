#include "HUD/HellwakeHUDWidget.h"
#include "Combat/HellwakeVitalityComponent.h"
#include "HellwakeEncounterSubsystem.h"
#include "Data/HellwakeLootDefinition.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Engine/DataTable.h"
#include "GameFramework/PlayerController.h"
#include "HellwakeGameplayTags.h"

void UHellwakeHUDWidget::BindToPlayerController(APlayerController* PC)
{
	if (!PC)
	{
		return;
	}

	APawn* Pawn = PC->GetPawn();
	if (UHellwakeVitalityComponent* Vitality = Pawn ? Pawn->FindComponentByClass<UHellwakeVitalityComponent>() : nullptr)
	{
		Vitality->OnHealthChanged.AddDynamic(this, &UHellwakeHUDWidget::HandleHealthChanged);
		Vitality->OnWrathChanged.AddDynamic(this, &UHellwakeHUDWidget::HandleWrathChanged);
		OnHealthChanged(Vitality->GetHealthPercent() * 100.f, 100.f);
		OnWrathChanged(Vitality->GetWrathPercent() * 100.f, 100.f);
	}

	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn))
	{
		if (UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			ASC->GenericGameplayEventCallbacks.FindOrAdd(HellwakeTags::Event_Loot_PickedUp)
				.AddUObject(this, &UHellwakeHUDWidget::HandleLootPickedUp);
		}
	}

	if (UWorld* World = GetWorld())
	{
		if (UHellwakeEncounterSubsystem* Encounter = World->GetSubsystem<UHellwakeEncounterSubsystem>())
		{
			Encounter->OnStageChanged.AddDynamic(this, &UHellwakeHUDWidget::HandleStageChanged);
			Encounter->OnBannerRequested.AddDynamic(this, &UHellwakeHUDWidget::HandleBannerRequested);
			OnObjectiveChanged(Encounter->GetObjectiveText(), Encounter->GetAliveHostileCount(), 0, 7);
			OnStageLabelChanged(Encounter->GetStageLabel());
		}
	}

	// NOTE: ability cooldown percents, boss HP/affixes, minimap blips, and
	// status pip countdowns are intentionally NOT wired here via delegates
	// — there isn't yet a single authoritative source for "all 5 ability
	// cooldowns" or "nearby enemy positions" the way hellwake-game.js's
	// single onHud(hud) payload was. The straightforward correct wiring
	// once abilities/enemies exist as real actors in a level:
	//   - Ability cooldowns: ASC->GetActiveGameplayEffects() filtered by
	//     each ability's CooldownTags, percent = TimeRemaining/Duration.
	//   - Boss: bind directly to the registered AHellwakeGravewarden's
	//     VitalityComponent + GetPhase(), once UHellwakeEncounterSubsystem
	//     exposes it (RegisterGravewarden already stores the pointer).
	//   - Minimap: query nearby AHellwakeEnemyBase/AHellwakeLootPickup via
	//     UGameplayStatics::GetAllActorsOfClass within minimap range each
	//     tick (cheap at slice scale) and convert to player-relative offsets.
	// See project-bible/hud.md for the full per-field wiring plan.
}

void UHellwakeHUDWidget::HandleHealthChanged(float NewValue, float MaxValue, float Delta)
{
	OnHealthChanged(NewValue, MaxValue);
}

void UHellwakeHUDWidget::HandleWrathChanged(float NewValue, float MaxValue, float Delta)
{
	OnWrathChanged(NewValue, MaxValue);
}

void UHellwakeHUDWidget::HandleStageChanged(EHellwakeEncounterStage NewStage)
{
	if (UWorld* World = GetWorld())
	{
		if (UHellwakeEncounterSubsystem* Encounter = World->GetSubsystem<UHellwakeEncounterSubsystem>())
		{
			OnObjectiveChanged(Encounter->GetObjectiveText(), Encounter->GetAliveHostileCount(), (int32)NewStage, 7);
			OnStageLabelChanged(Encounter->GetStageLabel());
		}
	}
	OnCinematicChanged(NewStage == EHellwakeEncounterStage::Intro);
}

void UHellwakeHUDWidget::HandleBannerRequested(const FText& BannerText)
{
	OnBannerTextChanged(BannerText);
}

void UHellwakeHUDWidget::HandleLootPickedUp(const FGameplayEventData* Payload)
{
	if (!Payload)
	{
		return;
	}
	const UDataTable* Table = Payload->OptionalObject ? Cast<UDataTable>(Payload->OptionalObject) : LootDefinitionTable.Get();
	// The pickup actor stashes its RarityRowName as the event's OptionalObject
	// table + the row lookup happens here; a production version should pass
	// the row name directly (e.g. via EventMagnitude as an index, or a
	// custom FGameplayEventData subclass) rather than re-deriving it.
	if (Table)
	{
		// See TODO in HellwakeLootPickup::OnPickupOverlap — RarityRowName
		// isn't currently threaded through FGameplayEventData. Wire a
		// concrete row name through before relying on this in production.
		OnLootToast(FText::GetEmpty(), FText::GetEmpty(), FLinearColor::White);
	}
}
