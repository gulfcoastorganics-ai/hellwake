#include "Loot/HellwakeLootPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Data/HellwakeLootDefinition.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "HellwakeGameplayTags.h"
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

	if (LootDefinitionTable && !RarityRowName.IsNone())
	{
		if (const FHellwakeLootDefinition* Row = LootDefinitionTable->FindRow<FHellwakeLootDefinition>(RarityRowName, TEXT("HellwakeLootPickup::BeginPlay")))
		{
			if (BeaconLight)
			{
				BeaconLight->SetLightColor(Row->Color);
			}
		}
	}
}

void AHellwakeLootPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// Prototype: `l.item.rotation.y += dt*1.6; l.item.position.y = 0.7 + sin(l.t*2)*0.12;`
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
	// TODO(HUD): the HUD widget should bind to Event.Loot.PickedUp on the
	// player's ASC and read RarityRowName (via a small helper reading
	// LootDefinitionTable) to show the 3.6s toast — see project-bible/hud.md.

	// TODO(VFX): burst(pos, def.c, 0.8, 18) on pickup.

	Destroy();
}
