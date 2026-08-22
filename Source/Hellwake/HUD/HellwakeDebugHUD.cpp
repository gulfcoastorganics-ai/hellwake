#include "HUD/HellwakeDebugHUD.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Attributes/HellwakeAttributeSet.h"
#include "Enemies/HellwakeGravewarden.h"
#include "HellwakeEncounterSubsystem.h"
#include "HellwakeGameplayTags.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"

void AHellwakeDebugHUD::DrawMeter(const FString& Label, float Current, float Max, float X, float Y, float Width, const FLinearColor& FillColor)
{
	const float SafeMax = FMath::Max(Max, 1.f);
	const float Percent = FMath::Clamp(Current / SafeMax, 0.f, 1.f);
	const float Height = 18.f;

	DrawRect(FLinearColor(0.02f, 0.02f, 0.025f, 0.9f), X, Y, Width, Height);
	DrawRect(FillColor, X + 2.f, Y + 2.f, (Width - 4.f) * Percent, Height - 4.f);
	DrawText(FString::Printf(TEXT("%s  %.0f / %.0f"), *Label, Current, Max), FLinearColor::White,
		X + 6.f, Y - 1.f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.9f, false);
}

void AHellwakeDebugHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas || !PlayerOwner)
	{
		return;
	}

	float Health = 0.f;
	float MaxHealth = 100.f;
	float Wrath = 0.f;
	float MaxWrath = 100.f;
	float XP = 0.f;
	UAbilitySystemComponent* PlayerASC = nullptr;

	if (APawn* Pawn = PlayerOwner->GetPawn())
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Pawn))
		{
			PlayerASC = ASI->GetAbilitySystemComponent();
			if (PlayerASC)
			{
				if (const UHellwakeAttributeSet* Attr = PlayerASC->GetSet<UHellwakeAttributeSet>())
				{
					Health = Attr->GetHealth();
					MaxHealth = Attr->GetMaxHealth();
					Wrath = Attr->GetWrath();
					MaxWrath = Attr->GetMaxWrath();
					XP = Attr->GetXP();
				}
			}
		}
	}

	const float Left = 34.f;
	const float Top = 34.f;
	const float MeterWidth = 300.f;
	DrawText(TEXT("KAERVOSS"), FLinearColor::White, Left, Top, GEngine ? GEngine->GetMediumFont() : nullptr, 1.f, false);
	DrawMeter(TEXT("HEALTH"), Health, MaxHealth, Left, Top + 34.f, MeterWidth, FLinearColor(0.62f, 0.08f, 0.05f, 1.f));
	DrawMeter(TEXT("WRATH"), Wrath, MaxWrath, Left, Top + 58.f, MeterWidth, FLinearColor(0.92f, 0.32f, 0.08f, 1.f));
	DrawMeter(TEXT("XP"), XP, 100.f, Left, Top + 82.f, MeterWidth, FLinearColor(0.48f, 0.38f, 0.74f, 1.f));

	if (PlayerASC)
	{
		TArray<FString> StatusLabels;
		if (PlayerASC->HasMatchingGameplayTag(HellwakeTags::Status_Ember)) StatusLabels.Add(TEXT("EMBER"));
		if (PlayerASC->HasMatchingGameplayTag(HellwakeTags::Status_Fortified)) StatusLabels.Add(TEXT("FORTIFIED"));
		if (PlayerASC->HasMatchingGameplayTag(HellwakeTags::State_IFrame)) StatusLabels.Add(TEXT("IFRAME"));
		if (PlayerASC->HasMatchingGameplayTag(HellwakeTags::State_Cinematic)) StatusLabels.Add(TEXT("CINEMATIC"));
		if (PlayerASC->HasMatchingGameplayTag(HellwakeTags::State_Dead)) StatusLabels.Add(TEXT("DEAD"));
		const FString StatusLine = StatusLabels.Num() > 0 ? FString::Join(StatusLabels, TEXT("  |  ")) : TEXT("STATUS: CLEAR");
		DrawText(StatusLine, FLinearColor(0.84f, 0.82f, 0.74f, 1.f), Left, Top + 110.f,
			GEngine ? GEngine->GetSmallFont() : nullptr, 0.86f, false);
	}

	FText Objective = FText::FromString(TEXT("Enter the Vaunhold plaza"));
	FText StageLabel = FText::FromString(TEXT("ENTERING PLAZA"));
	int32 AliveHostiles = 0;
	AHellwakeGravewarden* Gravewarden = nullptr;
	if (UWorld* World = GetWorld())
	{
		if (UHellwakeEncounterSubsystem* Encounter = World->GetSubsystem<UHellwakeEncounterSubsystem>())
		{
			Objective = Encounter->GetObjectiveText();
			StageLabel = Encounter->GetStageLabel();
			AliveHostiles = Encounter->GetAliveHostileCount();
			Gravewarden = Encounter->GetGravewarden();
		}
	}

	const float RightX = FMath::Max(Left + 360.f, Canvas->SizeX - 430.f);
	DrawText(StageLabel.ToString(), FLinearColor(0.95f, 0.72f, 0.36f, 1.f), RightX, Top,
		GEngine ? GEngine->GetMediumFont() : nullptr, 1.f, false);
	DrawText(Objective.ToString(), FLinearColor::White, RightX, Top + 30.f,
		GEngine ? GEngine->GetSmallFont() : nullptr, 1.f, false);
	DrawText(FString::Printf(TEXT("HOSTILES: %d"), AliveHostiles), FLinearColor::White, RightX, Top + 52.f,
		GEngine ? GEngine->GetSmallFont() : nullptr, 0.95f, false);

	if (IsValid(Gravewarden) && !Gravewarden->IsDead())
	{
		if (UAbilitySystemComponent* BossASC = Gravewarden->GetAbilitySystemComponent())
		{
			if (const UHellwakeAttributeSet* BossAttr = BossASC->GetSet<UHellwakeAttributeSet>())
			{
				const float BossWidth = FMath::Min(560.f, Canvas->SizeX * 0.42f);
				const float BossX = (Canvas->SizeX - BossWidth) * 0.5f;
				DrawText(TEXT("GRAVEWARDEN OF THE NINTH SEAL"), FLinearColor(0.95f, 0.72f, 0.36f, 1.f),
					BossX + 70.f, 28.f, GEngine ? GEngine->GetMediumFont() : nullptr, 0.95f, false);
				DrawMeter(TEXT("BOSS"), BossAttr->GetHealth(), BossAttr->GetMaxHealth(), BossX, 58.f, BossWidth,
					FLinearColor(0.42f, 0.03f, 0.025f, 1.f));
				DrawText(FString::Printf(TEXT("PHASE %d   ADDS %d"), static_cast<int32>(Gravewarden->GetPhase()) + 1, Gravewarden->GetAliveAddCount()),
					FLinearColor(0.85f, 0.75f, 0.62f, 1.f), BossX, 82.f, GEngine ? GEngine->GetSmallFont() : nullptr, 0.9f, false);
			}
		}
	}

	if (PlayerASC)
	{
		auto ReadyText = [PlayerASC](const TCHAR* Label, const FGameplayTag& CooldownTag)
		{
			return FString::Printf(TEXT("%s:%s"), Label, PlayerASC->HasMatchingGameplayTag(CooldownTag) ? TEXT("CD") : TEXT("READY"));
		};
		const FString AbilityLine = FString::Printf(TEXT("%s   %s   %s   %s   %s   %s   %s"),
			*ReadyText(TEXT("LMB"), HellwakeTags::Cooldown_LightAttack),
			*ReadyText(TEXT("RMB"), HellwakeTags::Cooldown_HeavyAttack),
			*ReadyText(TEXT("DODGE"), HellwakeTags::Cooldown_Dodge),
			*ReadyText(TEXT("Q"), HellwakeTags::Cooldown_Emberbrand),
			*ReadyText(TEXT("F"), HellwakeTags::Cooldown_Bulwark),
			*ReadyText(TEXT("E"), HellwakeTags::Cooldown_Ruinfall),
			*ReadyText(TEXT("R"), HellwakeTags::Cooldown_WakeOfHell));
		DrawText(AbilityLine, FLinearColor(0.95f, 0.72f, 0.36f, 1.f), 34.f, Canvas->SizeY - 68.f,
			GEngine ? GEngine->GetSmallFont() : nullptr, 0.84f, false);
	}

	const FString Controls = TEXT("WASD Move   LMB Light   RMB Heavy   Shift/Space Dodge   Q Emberbrand   F Bulwark   E Ruinfall   R Wake of Hell");
	DrawText(Controls, FLinearColor(0.8f, 0.8f, 0.82f, 1.f), 34.f, Canvas->SizeY - 44.f,
		GEngine ? GEngine->GetSmallFont() : nullptr, 0.85f, false);
}
