// A dropped item in the world. Prototype: dropLoot() spawns an octahedron +
// light beam + ground pool; `hero.position.distanceTo(l.obj.position) < 2.2`
// auto-picks it up (no button press, no inventory check — it's instant and
// unconditional). Ported as-is: overlap with the player triggers pickup.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HellwakeLootPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class UPrimitiveComponent;
class UDataTable;

UCLASS()
class HELLWAKE_API AHellwakeLootPickup : public AActor
{
	GENERATED_BODY()

public:
	AHellwakeLootPickup();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// Safe to call immediately after SpawnActor. Updates both data fields and
	// the asset-free visual fallback, so native drops do not depend on a
	// Blueprint construction script.
	UFUNCTION(BlueprintCallable, Category = "Hellwake|Loot")
	void ConfigurePickup(FName InRarityRowName, UDataTable* InLootDefinitionTable = nullptr);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake|Loot")
	FName RarityRowName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Loot")
	TObjectPtr<UDataTable> LootDefinitionTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Loot")
	float PickupRadiusCm = 220.f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Loot")
	TObjectPtr<USphereComponent> PickupSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Loot")
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Loot")
	TObjectPtr<UPointLightComponent> BeaconLight;

private:
	float BobTime = 0.f;

	UFUNCTION()
	void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
