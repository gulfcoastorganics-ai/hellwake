// Owns the parts of the prototype's camera behavior that are dynamic
// per-frame state rather than the fixed boom setup (that part lives on
// HellwakeCharacter's CameraBoom/TopDownCamera). Ports:
//
//   const zoomWant = cine>0 ? 0.82 : bossActive ? 1.12 : 1;
//   camZoom += (zoomWant - camZoom) * min(1, 3*dt);
//   shake = max(0, shake - dt*2.4);
//   camera offset += random jitter scaled by `shake`
//   if (hitStop>0) { hitStop -= dt; dt *= 0.12; }
//
// Attach to the player character. TriggerShake/TriggerHitStop are called
// from abilities and from enemy/boss attack-land events.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HellwakeCameraDirector.generated.h"

class USpringArmComponent;
class UCameraShakeBase;

UCLASS(ClassGroup = (Hellwake), meta = (BlueprintSpawnableComponent))
class HELLWAKE_API UHellwakeCameraDirector : public UActorComponent
{
	GENERATED_BODY()

public:
	UHellwakeCameraDirector();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Prototype: shake = max(shake, X) — new triggers only raise the level,
	// they don't reset an already-higher ongoing shake.
	UFUNCTION(BlueprintCallable, Category = "Hellwake|Camera")
	void TriggerShake(float Magnitude);

	// Prototype: hitStop = max(hitStop, Seconds); dt *= 0.12 while active.
	// Implemented as a brief global time-dilation dip (restored automatically)
	// since the prototype's single JS loop scales dt for the whole
	// simulation (player, enemies, VFX), not just the camera.
	UFUNCTION(BlueprintCallable, Category = "Hellwake|Camera")
	void TriggerHitStop(float Seconds);

	// Drives zoomWant. Set true while `stage() === 'boss' && warden alive`.
	UFUNCTION(BlueprintCallable, Category = "Hellwake|Camera")
	void SetBossFightActive(bool bActive) { bBossFightActive = bActive; }

	// Drives zoomWant. Set true while `cine > 0` (boss intro/death, Wake of
	// Hell cast) — same tag-driven source as State.Cinematic.
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

	// Applied as: BoomLength = BaseArmLength * CurrentZoom.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Camera")
	float BaseArmLength = 3700.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Camera")
	float ShakeSocketOffsetScale = 40.f;

	// Time dilation applied for the duration of a hit-stop, matching the
	// prototype's `dt *= 0.12`.
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
	bool bHitStopApplied = false;
};
