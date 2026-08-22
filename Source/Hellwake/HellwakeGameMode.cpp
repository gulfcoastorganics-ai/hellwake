#include "HellwakeGameMode.h"
#include "HellwakeEncounterSubsystem.h"
#include "Data/HellwakeEncounterStage.h"
#include "Data/HellwakeEnemyDefinition.h"
#include "Enemies/HellwakeEnemyBase.h"
#include "Enemies/HellwakeGravewarden.h"
#include "HUD/HellwakeDebugHUD.h"
#include "Player/HellwakeCharacter.h"
#include "Player/HellwakePlayerController.h"
#include "Components/LightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMeshActor.h"
#include "HellwakeGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Hellwake.h"

namespace
{
	struct FFallbackSpawn
	{
		FName RowName;
		FVector Location;
	};

	FHellwakeEnemyDefinition MakeFallbackEnemyDefinition(const FName RowName)
	{
		FHellwakeEnemyDefinition Def;
		if (RowName == TEXT("reaver"))
		{
			Def.DisplayName = FText::FromString(TEXT("Ashbound Reaver"));
			Def.RoleTag = HellwakeTags::Enemy_Role_Reaver;
			Def.MaxHealth = 180.f;
			Def.MoveSpeedCm = 440.f;
			Def.AttackDamage = 11.f;
			Def.AttackRangeCm = 290.f;
			Def.AttackCooldownSeconds = 1.7f;
			Def.EngagementRingCm = 320.f;
			Def.LootDropChance = 0.35f;
			Def.XPReward = 40.f;
		}
		else if (RowName == TEXT("wraith"))
		{
			Def.DisplayName = FText::FromString(TEXT("Cinder Wraith"));
			Def.RoleTag = HellwakeTags::Enemy_Role_Wraith;
			Def.MaxHealth = 110.f;
			Def.MoveSpeedCm = 720.f;
			Def.AttackDamage = 7.f;
			Def.AttackRangeCm = 240.f;
			Def.AttackCooldownSeconds = 1.1f;
			Def.EngagementRingCm = 260.f;
			Def.LootDropChance = 0.30f;
			Def.XPReward = 32.f;
		}
		else if (RowName == TEXT("acolyte"))
		{
			Def.DisplayName = FText::FromString(TEXT("Pyre Acolyte"));
			Def.RoleTag = HellwakeTags::Enemy_Role_Acolyte;
			Def.MaxHealth = 130.f;
			Def.MoveSpeedCm = 340.f;
			Def.AttackDamage = 14.f;
			Def.AttackRangeCm = 1700.f;
			Def.AttackCooldownSeconds = 2.6f;
			Def.EngagementRingCm = 1300.f;
			Def.LootDropChance = 0.45f;
			Def.XPReward = 46.f;
		}
		else
		{
			Def.DisplayName = FText::FromString(TEXT("Gravewarden of the Ninth Seal"));
			Def.RoleTag = HellwakeTags::Enemy_Role_Gravewarden;
			Def.MaxHealth = 3200.f;
			Def.MoveSpeedCm = 400.f;
			Def.AttackDamage = 26.f;
			Def.AttackRangeCm = 540.f;
			Def.AttackCooldownSeconds = 2.4f;
			Def.EngagementRingCm = 500.f;
			Def.LootDropChance = 1.f;
			Def.XPReward = 900.f;
			Def.bIsElite = true;
		}
		return Def;
	}

	TArray<FFallbackSpawn> MakeFallbackSpawns(const EHellwakeEncounterStage Stage)
	{
		switch (Stage)
		{
		case EHellwakeEncounterStage::Wave1:
			return {
				{ TEXT("reaver"), FVector(-500.f, 600.f, 100.f) },
				{ TEXT("reaver"), FVector(500.f, 700.f, 100.f) },
				{ TEXT("wraith"), FVector(0.f, 200.f, 100.f) },
				{ TEXT("acolyte"), FVector(-1200.f, -200.f, 100.f) }
			};
		case EHellwakeEncounterStage::Wave2:
			return {
				{ TEXT("reaver"), FVector(-800.f, -800.f, 100.f) },
				{ TEXT("reaver"), FVector(900.f, -700.f, 100.f) },
				{ TEXT("wraith"), FVector(-300.f, -1200.f, 100.f) },
				{ TEXT("wraith"), FVector(600.f, -1300.f, 100.f) },
				{ TEXT("acolyte"), FVector(1300.f, -1400.f, 100.f) }
			};
		case EHellwakeEncounterStage::Intro:
			return { { TEXT("gravewarden"), FVector(0.f, -2200.f, 100.f) } };
		default:
			return {};
		}
	}

	FName StageToRowName(const EHellwakeEncounterStage Stage)
	{
		switch (Stage)
		{
		case EHellwakeEncounterStage::Wave1: return TEXT("wave1");
		case EHellwakeEncounterStage::Wave2: return TEXT("wave2");
		case EHellwakeEncounterStage::Intro: return TEXT("intro");
		default: return NAME_None;
		}
	}
}

AHellwakeGameMode::AHellwakeGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = AHellwakeCharacter::StaticClass();
	PlayerControllerClass = AHellwakePlayerController::StaticClass();
	HUDClass = AHellwakeDebugHUD::StaticClass();

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded())
	{
		ArenaCubeMesh = CubeFinder.Object;
	}
}

void AHellwakeGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (bBuildNativeFallbackArena)
	{
		BuildNativeFallbackArena();
	}

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

void AHellwakeGameMode::BuildNativeFallbackArena()
{
	if (!GetWorld() || !ArenaCubeMesh)
	{
		return;
	}

	auto SpawnBlock = [this](const FVector& Location, const FVector& Scale)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AStaticMeshActor* Block = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator, Params);
		if (!Block)
		{
			return;
		}
		UStaticMeshComponent* Mesh = Block->GetStaticMeshComponent();
		Mesh->SetMobility(EComponentMobility::Movable);
		Mesh->SetStaticMesh(ArenaCubeMesh);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Block->SetActorScale3D(Scale);
	};

	// 100 cm engine cube: floor top is -100 cm, just below the default
	// character capsule bottom. Walls frame the ~60 m prototype run from
	// the plaza entrance through the boss arena.
	SpawnBlock(FVector(0.f, -300.f, -200.f), FVector(40.f, 60.f, 2.f));
	SpawnBlock(FVector(-2050.f, -300.f, 300.f), FVector(1.f, 60.f, 8.f));
	SpawnBlock(FVector(2050.f, -300.f, 300.f), FVector(1.f, 60.f, 8.f));
	SpawnBlock(FVector(0.f, 2650.f, 300.f), FVector(40.f, 1.f, 8.f));
	SpawnBlock(FVector(0.f, -3250.f, 300.f), FVector(40.f, 1.f, 8.f));

	FActorSpawnParameters LightParams;
	LightParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (ADirectionalLight* KeyLight = GetWorld()->SpawnActor<ADirectionalLight>(FVector::ZeroVector, FRotator(-52.f, -32.f, 0.f), LightParams))
	{
		if (ULightComponent* Light = KeyLight->GetLightComponent())
		{
			Light->SetIntensity(8.f);
		}
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
	if (AHellwakeCharacter* Player = Cast<AHellwakeCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		switch (NewStage)
		{
		case EHellwakeEncounterStage::Intro:
			Player->SetBossFightCameraActive(false);
			Player->BeginCinematic(3.4f);
			break;
		case EHellwakeEncounterStage::Boss:
			Player->EndCinematic();
			Player->SetBossFightCameraActive(true);
			break;
		case EHellwakeEncounterStage::Reward:
		case EHellwakeEncounterStage::Done:
			Player->EndCinematic();
			Player->SetBossFightCameraActive(false);
			break;
		default:
			break;
		}
	}

	SpawnEnemiesForStage(NewStage);
}

void AHellwakeGameMode::SpawnEnemiesForStage(EHellwakeEncounterStage Stage)
{
	if (!Encounter || !GetWorld())
	{
		return;
	}

	TArray<FFallbackSpawn> Spawns;
	const FName StageRowName = StageToRowName(Stage);
	if (!StageRowName.IsNone() && Encounter->EncounterStageTable)
	{
		if (const FHellwakeEncounterStageDefinition* StageRow = Encounter->EncounterStageTable->FindRow<FHellwakeEncounterStageDefinition>(StageRowName, TEXT("SpawnEnemiesForStage")))
		{
			const int32 Count = FMath::Min(StageRow->SpawnRowNames.Num(), StageRow->SpawnLocations.Num());
			for (int32 i = 0; i < Count; ++i)
			{
				FVector Location = StageRow->SpawnLocations[i];
				if (Location.Z < 50.f)
				{
					Location.Z = 100.f;
				}
				Spawns.Add({ StageRow->SpawnRowNames[i], Location });
			}
		}
	}

	if (Spawns.Num() == 0)
	{
		Spawns = MakeFallbackSpawns(Stage);
	}
	if (Spawns.Num() == 0)
	{
		return;
	}

	TArray<AHellwakeEnemyBase*> SpawnedWave;
	for (const FFallbackSpawn& Spawn : Spawns)
	{
		FHellwakeEnemyDefinition FallbackDef = MakeFallbackEnemyDefinition(Spawn.RowName);
		const FHellwakeEnemyDefinition* EnemyDef = &FallbackDef;
		if (EnemyDefinitionTable)
		{
			if (const FHellwakeEnemyDefinition* DataRow = EnemyDefinitionTable->FindRow<FHellwakeEnemyDefinition>(Spawn.RowName, TEXT("SpawnEnemiesForStage")))
			{
				EnemyDef = DataRow;
			}
		}

		UClass* PawnClass = nullptr;
		if (!EnemyDef->PawnClass.IsNull())
		{
			PawnClass = EnemyDef->PawnClass.LoadSynchronous();
		}
		if (!PawnClass)
		{
			PawnClass = (Spawn.RowName == TEXT("gravewarden") || EnemyDef->RoleTag == HellwakeTags::Enemy_Role_Gravewarden)
				? AHellwakeGravewarden::StaticClass()
				: AHellwakeEnemyBase::StaticClass();
		}

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(PawnClass, Spawn.Location, FRotator::ZeroRotator, Params);
		AHellwakeEnemyBase* Enemy = Cast<AHellwakeEnemyBase>(SpawnedActor);
		if (!Enemy)
		{
			UE_LOG(LogHellwake, Warning, TEXT("SpawnEnemiesForStage: class for '%s' is not AHellwakeEnemyBase"), *Spawn.RowName.ToString());
			continue;
		}

		Enemy->InitializeFromDefinition(*EnemyDef, Spawn.RowName);
		if (AHellwakeGravewarden* Warden = Cast<AHellwakeGravewarden>(Enemy))
		{
			Encounter->RegisterGravewarden(Warden);
		}
		else
		{
			SpawnedWave.Add(Enemy);
		}
	}

	if (SpawnedWave.Num() > 0)
	{
		Encounter->RegisterActiveWaveEnemies(SpawnedWave);
	}
}
