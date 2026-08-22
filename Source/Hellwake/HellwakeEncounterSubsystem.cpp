#include "HellwakeEncounterSubsystem.h"
#include "Enemies/HellwakeEnemyBase.h"
#include "Enemies/HellwakeGravewarden.h"
#include "Engine/DataTable.h"
#include "Hellwake.h"

namespace
{
	FText FallbackObjective(const EHellwakeEncounterStage Stage)
	{
		switch (Stage)
		{
		case EHellwakeEncounterStage::Enter: return FText::FromString(TEXT("Enter the Vaunhold plaza"));
		case EHellwakeEncounterStage::Wave1: return FText::FromString(TEXT("Break the first procession"));
		case EHellwakeEncounterStage::Advance: return FText::FromString(TEXT("Advance to the cathedral steps"));
		case EHellwakeEncounterStage::Wave2: return FText::FromString(TEXT("Clear the ritual guard"));
		case EHellwakeEncounterStage::Intro: return FText::FromString(TEXT("The Gravewarden wakes"));
		case EHellwakeEncounterStage::Boss: return FText::FromString(TEXT("Slay the Gravewarden"));
		case EHellwakeEncounterStage::Reward: return FText::FromString(TEXT("Claim the Last Vow"));
		case EHellwakeEncounterStage::Done: return FText::FromString(TEXT("Vertical slice complete"));
		default: return FText::GetEmpty();
		}
	}

	FText FallbackStageLabel(const EHellwakeEncounterStage Stage)
	{
		switch (Stage)
		{
		case EHellwakeEncounterStage::Enter: return FText::FromString(TEXT("ENTERING PLAZA"));
		case EHellwakeEncounterStage::Wave1: return FText::FromString(TEXT("ENCOUNTER 1"));
		case EHellwakeEncounterStage::Advance: return FText::FromString(TEXT("ADVANCE"));
		case EHellwakeEncounterStage::Wave2: return FText::FromString(TEXT("ENCOUNTER 2"));
		case EHellwakeEncounterStage::Intro: return FText::FromString(TEXT("ELITE INTRO"));
		case EHellwakeEncounterStage::Boss: return FText::FromString(TEXT("GRAVEWARDEN"));
		case EHellwakeEncounterStage::Reward: return FText::FromString(TEXT("LEGENDARY DROP"));
		case EHellwakeEncounterStage::Done: return FText::FromString(TEXT("SLICE COMPLETE"));
		default: return FText::GetEmpty();
		}
	}

	FText FallbackBanner(const EHellwakeEncounterStage Stage)
	{
		switch (Stage)
		{
		case EHellwakeEncounterStage::Enter: return FText::FromString(TEXT("ENTER THE VAUNHOLD PLAZA"));
		case EHellwakeEncounterStage::Wave1: return FText::FromString(TEXT("BREAK THE FIRST PROCESSION"));
		case EHellwakeEncounterStage::Advance: return FText::FromString(TEXT("ADVANCE TO THE CATHEDRAL STEPS"));
		case EHellwakeEncounterStage::Wave2: return FText::FromString(TEXT("CLEAR THE RITUAL GUARD"));
		case EHellwakeEncounterStage::Intro: return FText::FromString(TEXT("THE GRAVEWARDEN WAKES"));
		case EHellwakeEncounterStage::Boss: return FText::FromString(TEXT("SLAY THE GRAVEWARDEN"));
		case EHellwakeEncounterStage::Reward: return FText::FromString(TEXT("THE NINTH SEAL BREAKS"));
		case EHellwakeEncounterStage::Done: return FText::FromString(TEXT("VERTICAL SLICE COMPLETE"));
		default: return FText::GetEmpty();
		}
	}
}

bool UHellwakeEncounterSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHellwakeEncounterSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UHellwakeEncounterSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UHellwakeEncounterSubsystem::BeginEncounter()
{
	CurrentStage = EHellwakeEncounterStage::Enter;
	StageTimer = 0.f;
	bLegendaryLootTaken = false;
	TrackedWaveEnemies.Reset();
	TrackedGravewarden = nullptr;
	SetStage(EHellwakeEncounterStage::Enter);
}

FName UHellwakeEncounterSubsystem::StageToRowName(EHellwakeEncounterStage Stage)
{
	switch (Stage)
	{
	case EHellwakeEncounterStage::Enter: return TEXT("enter");
	case EHellwakeEncounterStage::Wave1: return TEXT("wave1");
	case EHellwakeEncounterStage::Advance: return TEXT("advance");
	case EHellwakeEncounterStage::Wave2: return TEXT("wave2");
	case EHellwakeEncounterStage::Intro: return TEXT("intro");
	case EHellwakeEncounterStage::Boss: return TEXT("boss");
	case EHellwakeEncounterStage::Reward: return TEXT("reward");
	case EHellwakeEncounterStage::Done: return TEXT("done");
	default: return NAME_None;
	}
}

const FHellwakeEncounterStageDefinition* UHellwakeEncounterSubsystem::GetRow(EHellwakeEncounterStage Stage) const
{
	if (!EncounterStageTable)
	{
		return nullptr;
	}
	return EncounterStageTable->FindRow<FHellwakeEncounterStageDefinition>(StageToRowName(Stage), TEXT("UHellwakeEncounterSubsystem::GetRow"));
}

void UHellwakeEncounterSubsystem::SetStage(EHellwakeEncounterStage NewStage)
{
	CurrentStage = NewStage;
	StageTimer = 0.f;

	const FHellwakeEncounterStageDefinition* Row = GetRow(NewStage);
	const FText StageLabel = Row ? Row->StageLabel : FallbackStageLabel(NewStage);
	const FText Banner = Row ? Row->BannerText : FallbackBanner(NewStage);
	UE_LOG(LogHellwake, Log, TEXT("Encounter stage -> %s"), *StageLabel.ToString());

	OnStageChanged.Broadcast(NewStage);
	if (!Banner.IsEmpty())
	{
		OnBannerRequested.Broadcast(Banner);
	}
}

void UHellwakeEncounterSubsystem::Tick(float DeltaSeconds, const FVector& PlayerLocation)
{
	StageTimer += DeltaSeconds;

	TrackedWaveEnemies.RemoveAll([](const TObjectPtr<AHellwakeEnemyBase>& E) { return !IsValid(E); });
	const int32 AliveCount = GetAliveHostileCount();

	switch (CurrentStage)
	{
	case EHellwakeEncounterStage::Enter:
		if (PlayerLocation.Y < 1200.f)
		{
			SetStage(EHellwakeEncounterStage::Wave1);
		}
		break;
	case EHellwakeEncounterStage::Wave1:
		if (AliveCount == 0 && StageTimer > 1.f)
		{
			SetStage(EHellwakeEncounterStage::Advance);
		}
		break;
	case EHellwakeEncounterStage::Advance:
		if (PlayerLocation.Y < -400.f)
		{
			SetStage(EHellwakeEncounterStage::Wave2);
		}
		break;
	case EHellwakeEncounterStage::Wave2:
		if (AliveCount == 0 && StageTimer > 1.f)
		{
			SetStage(EHellwakeEncounterStage::Intro);
		}
		break;
	case EHellwakeEncounterStage::Intro:
		if (StageTimer > 3.4f)
		{
			SetStage(EHellwakeEncounterStage::Boss);
		}
		break;
	case EHellwakeEncounterStage::Boss:
		if (TrackedGravewarden && TrackedGravewarden->IsDead() && StageTimer > 2.2f)
		{
			SetStage(EHellwakeEncounterStage::Reward);
		}
		break;
	case EHellwakeEncounterStage::Reward:
		if (bLegendaryLootTaken && StageTimer > 1.f)
		{
			SetStage(EHellwakeEncounterStage::Done);
		}
		break;
	case EHellwakeEncounterStage::Done:
	default:
		break;
	}
}

FText UHellwakeEncounterSubsystem::GetObjectiveText() const
{
	const FHellwakeEncounterStageDefinition* Row = GetRow(CurrentStage);
	return Row ? Row->ObjectiveText : FallbackObjective(CurrentStage);
}

FText UHellwakeEncounterSubsystem::GetStageLabel() const
{
	const FHellwakeEncounterStageDefinition* Row = GetRow(CurrentStage);
	return Row ? Row->StageLabel : FallbackStageLabel(CurrentStage);
}

int32 UHellwakeEncounterSubsystem::GetAliveHostileCount() const
{
	int32 Count = 0;
	for (const TObjectPtr<AHellwakeEnemyBase>& Enemy : TrackedWaveEnemies)
	{
		if (IsValid(Enemy) && !Enemy->IsDead())
		{
			++Count;
		}
	}
	return Count;
}

void UHellwakeEncounterSubsystem::RegisterActiveWaveEnemies(const TArray<AHellwakeEnemyBase*>& Enemies)
{
	TrackedWaveEnemies.Reset();
	for (AHellwakeEnemyBase* Enemy : Enemies)
	{
		if (Enemy)
		{
			TrackedWaveEnemies.Add(Enemy);
		}
	}
}
