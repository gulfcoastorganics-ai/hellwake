#include "HellwakeGameMode.h"
#include "HellwakeEncounterSubsystem.h"
#include "Data/HellwakeEncounterStage.h"
#include "Data/HellwakeEnemyDefinition.h"
#include "Enemies/HellwakeEnemyBase.h"
#include "Enemies/HellwakeGravewarden.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Hellwake.h"

AHellwakeGameMode::AHellwakeGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AHellwakeGameMode::BeginPlay()
{
	Super::BeginPlay();

	Encounter = GetWorld()->GetSubsystem<UHellwakeEncounterSubsystem>();
	if (Encounter)
	{
		Encounter->OnStageChanged.AddDynamic(this, &AHellwakeGameMode::HandleStageChanged);
		Encounter->BeginEncounter();
	}
	else
	{
		UE_LOG(LogHellwake, Error, TEXT("AHellwakeGameMode: UHellwakeEncounterSubsystem missing from world."));
	}
}

void AHellwakeGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!Encounter)
	{
		return;
	}
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		Encounter->Tick(DeltaSeconds, PlayerPawn->GetActorLocation());
	}
}

void AHellwakeGameMode::HandleStageChanged(EHellwakeEncounterStage NewStage)
{
	SpawnEnemiesForStage(NewStage);
}

void AHellwakeGameMode::SpawnEnemiesForStage(EHellwakeEncounterStage Stage)
{
	if (!Encounter || !Encounter->EncounterStageTable || !EnemyDefinitionTable || !GetWorld())
	{
		return;
	}

	static const auto RowNameFor = [](EHellwakeEncounterStage S) -> FName
	{
		switch (S)
		{
		case EHellwakeEncounterStage::Wave1: return TEXT("wave1");
		case EHellwakeEncounterStage::Wave2: return TEXT("wave2");
		case EHellwakeEncounterStage::Intro: return TEXT("intro");
		default: return NAME_None;
		}
	};

	const FName RowName = RowNameFor(Stage);
	if (RowName.IsNone())
	{
		return;
	}
	const FHellwakeEncounterStageDefinition* StageRow = Encounter->EncounterStageTable->FindRow<FHellwakeEncounterStageDefinition>(RowName, TEXT("SpawnEnemiesForStage"));
	if (!StageRow)
	{
		return;
	}

	TArray<AHellwakeEnemyBase*> Spawned;
	const int32 Count = FMath::Min(StageRow->SpawnRowNames.Num(), StageRow->SpawnLocations.Num());
	for (int32 i = 0; i < Count; ++i)
	{
		const FHellwakeEnemyDefinition* EnemyRow = EnemyDefinitionTable->FindRow<FHellwakeEnemyDefinition>(StageRow->SpawnRowNames[i], TEXT("SpawnEnemiesForStage"));
		if (!EnemyRow || EnemyRow->PawnClass.IsNull())
		{
			UE_LOG(LogHellwake, Warning, TEXT("SpawnEnemiesForStage: no PawnClass for row '%s'"), *StageRow->SpawnRowNames[i].ToString());
			continue;
		}

		UClass* PawnClass = EnemyRow->PawnClass.LoadSynchronous();
		if (!PawnClass)
		{
			continue;
		}

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(PawnClass, StageRow->SpawnLocations[i], FRotator::ZeroRotator, Params);

		if (AHellwakeGravewarden* Warden = Cast<AHellwakeGravewarden>(SpawnedActor))
		{
			Encounter->RegisterGravewarden(Warden);
		}
		else if (AHellwakeEnemyBase* Enemy = Cast<AHellwakeEnemyBase>(SpawnedActor))
		{
			Enemy->InitializeFromDefinition(*EnemyRow, StageRow->SpawnRowNames[i]);
			Spawned.Add(Enemy);
		}
	}

	if (Spawned.Num() > 0)
	{
		Encounter->RegisterActiveWaveEnemies(Spawned);
	}
}
