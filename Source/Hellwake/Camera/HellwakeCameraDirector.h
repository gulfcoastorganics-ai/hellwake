// Owns the dynamic fixed-isometric camera behavior: encounter zoom,
// decaying shake, and short global hit-stop windows.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HellwakeCameraDirector.generated.h"

class USpringArmComponent;

UCLASS(ClassGroup = (Hellwake), meta = (BlueprintSpawnableComponent))
class HELLWAKE_API UHellwakeCameraDirector : public UActorComponent
{
	GENERATED_BODY()

public:
	UHellwakeCameraDirector();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Hellwake|Camera")
	void TriggerShake(float Magnitude);

	UFUNCTION(BlueprintCallable, Category = "Hellwake|Camera")
	void TriggerHitStop(float Seconds);

	UFUNCTION(BlueprintCallable, Category = "Hellwake|Camera")
	void SetBossFightActive(bool bActive) { bBossFightActive = bActive; }

	UFUNCTION(BlueprintCallable, Category = "Hellwake|Camera")
	void SetCinematicActive(bool bActive) { bCinematicActive = bActive; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Camera")
	float CinematicZoom = 0.82f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Camera")
	float BossFightZoom = 1.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Camera")
	float ZoomLerpRate = 3.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Camera")
	float ShakeDecayPerSecond = 2.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Camera")
	float BaseArmLength = 3700.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Camera")
	float ShakeSocketOffsetScale = 40.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Camera")
	float HitStopTimeDilation = 0.12f;

private:
	UPROPERTY()
	TObjectPtr<USpringArmComponent> CachedBoom;

	bool bBossFightActive = false;
	bool bCinematicActive = false;
	float CurrentZoom = 1.f;
	float ShakeLevel = 0.f;
	float HitStopRemaining = 0.f;
	float PreviousGlobalTimeDilation = 1.f;
	bool bHitStopApplied = false;
};
