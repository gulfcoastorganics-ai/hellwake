#include "Loot/HellwakeLootPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Data/HellwakeLootDefinition.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "HellwakeEncounterSubsystem.h"
#include "HellwakeGameplayTags.h"
#include "UObject/ConstructorHelpers.h"
#include "Hellwake.h"

AHellwakeLootPickup::AHellwakeLootPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->InitSphereRadius(PickupRadiusCm);
	PickupSphere->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	RootComponent = PickupSphere;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	ItemMesh->SetupAttachment(RootComponent);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ItemMesh->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
	ItemMesh->SetRelativeScale3D(FVector(0.38f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereFinder.Succeeded())
	{
		ItemMesh->SetStaticMesh(SphereFinder.Object);
	}

	BeaconLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BeaconLight"));
	BeaconLight->SetupAttachment(RootComponent);
	BeaconLight->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	BeaconLight->Intensity = 2600.f;
	BeaconLight->AttenuationRadius = 1000.f;
}

void AHellwakeLootPickup::BeginPlay()
{
	Super::BeginPlay();
	PickupSphere->SetSphereRadius(PickupRadiusCm);
	PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AHellwakeLootPickup::OnPickupOverlap);

	if (!RarityRowName.IsNone())
	{
		ConfigurePickup(RarityRowName, LootDefinitionTable);
	}
}

void AHellwakeLootPickup::ConfigurePickup(FName InRarityRowName, UDataTable* InLootDefinitionTable)
{
	RarityRowName = InRarityRowName;
	if (InLootDefinitionTable)
	{
		LootDefinitionTable = InLootDefinitionTable;
	}

	FLinearColor RarityColor(0.85f, 0.85f, 0.88f, 1.f);
	bool bResolvedFromTable = false;
	if (LootDefinitionTable && !RarityRowName.IsNone())
	{
		if (const FHellwakeLootDefinition* Row = LootDefinitionTable->FindRow<FHellwakeLootDefinition>(RarityRowName, TEXT("HellwakeLootPickup::ConfigurePickup")))
		{
			RarityColor = Row->Color;
			bResolvedFromTable = true;
		}
	}

	if (!bResolvedFromTable)
	{
		if (RarityRowName == TEXT("legendary"))
		{
			RarityColor = FLinearColor(1.f, 0.48f, 0.08f, 1.f);
		}
		else if (RarityRowName == TEXT("rare"))
		{
			RarityColor = FLinearColor(0.62f, 0.30f, 0.95f, 1.f);
		}
		else if (RarityRowName == TEXT("magic"))
		{
			RarityColor = FLinearColor(0.20f, 0.52f, 1.f, 1.f);
		}
	}

	if (BeaconLight)
	{
		BeaconLight->SetLightColor(RarityColor);
		BeaconLight->SetIntensity(RarityRowName == TEXT("legendary") ? 5200.f : 2600.f);
	}
	if (ItemMesh)
	{
		ItemMesh->SetRelativeScale3D(RarityRowName == TEXT("legendary") ? FVector(0.58f) : FVector(0.38f));
	}
}

void AHellwakeLootPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	BobTime += DeltaSeconds;
	if (ItemMesh)
	{
		ItemMesh->AddLocalRotation(FRotator(0.f, DeltaSeconds * 90.f, 0.f));
		ItemMesh->SetRelativeLocation(FVector(0.f, 0.f, 70.f + FMath::Sin(BobTime * 2.f) * 12.f));
	}
}

void AHellwakeLootPickup::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACharacter* PlayerCharacter = Cast<ACharacter>(OtherActor);
	if (!PlayerCharacter || !PlayerCharacter->IsPlayerControlled())
	{
		return;
	}

	UE_LOG(LogHellwake, Log, TEXT("%s picked up loot '%s'"), *PlayerCharacter->GetName(), *RarityRowName.ToString());

	FGameplayEventData Payload;
	Payload.EventTag = HellwakeTags::Event_Loot_PickedUp;
	Payload.Instigator = this;
	Payload.Target = PlayerCharacter;
	Payload.OptionalObject = LootDefinitionTable;
	Payload.ContextHandle.AddInstigator(this, this);
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(PlayerCharacter, HellwakeTags::Event_Loot_PickedUp, Payload);

	if (RarityRowName == TEXT("legendary"))
	{
		if (UWorld* World = GetWorld())
		{
			if (UHellwakeEncounterSubsystem* Encounter = World->GetSubsystem<UHellwakeEncounterSubsystem>())
			{
				Encounter->NotifyLegendaryLootTaken();
			}
		}
	}

	Destroy();
}
