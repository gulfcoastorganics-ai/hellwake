#include "HellwakeEncounterSubsystem.h"
#include "Enemies/HellwakeEnemyBase.h"
#include "Enemies/HellwakeGravewarden.h"
#include "Engine/DataTable.h"
#include "Hellwake.h"

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
	UE_LOG(LogHellwake, Log, TEXT("Encounter stage -> %s"), Row ? *Row->StageLabel.ToString() : *StageToRowName(NewStage).ToString());

	OnStageChanged.Broadcast(NewStage);
	if (Row)
	{
		OnBannerRequested.Broadcast(Row->BannerText);
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
	return Row ? Row->ObjectiveText : FText::GetEmpty();
}

FText UHellwakeEncounterSubsystem::GetStageLabel() const
{
	const FHellwakeEncounterStageDefinition* Row = GetRow(CurrentStage);
	return Row ? Row->StageLabel : FText::GetEmpty();
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
