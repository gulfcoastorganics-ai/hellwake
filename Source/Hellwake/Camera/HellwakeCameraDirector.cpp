#include "Camera/HellwakeCameraDirector.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/App.h"

UHellwakeCameraDirector::UHellwakeCameraDirector()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHellwakeCameraDirector::BeginPlay()
{
	Super::BeginPlay();
	if (AActor* Owner = GetOwner())
	{
		CachedBoom = Owner->FindComponentByClass<USpringArmComponent>();
		if (CachedBoom)
		{
			BaseArmLength = CachedBoom->TargetArmLength;
		}
	}
}

void UHellwakeCameraDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bHitStopApplied && GetWorld())
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), PreviousGlobalTimeDilation);
		bHitStopApplied = false;
	}
	Super::EndPlay(EndPlayReason);
}

void UHellwakeCameraDirector::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const float ZoomWant = bCinematicActive ? CinematicZoom : (bBossFightActive ? BossFightZoom : 1.f);
	CurrentZoom += (ZoomWant - CurrentZoom) * FMath::Min(1.f, ZoomLerpRate * DeltaTime);

	ShakeLevel = FMath::Max(0.f, ShakeLevel - DeltaTime * ShakeDecayPerSecond);

	if (CachedBoom)
	{
		CachedBoom->TargetArmLength = BaseArmLength * CurrentZoom;
		const FVector Jitter = FVector(
			FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f)) * ShakeLevel * ShakeSocketOffsetScale;
		CachedBoom->SocketOffset = Jitter;
	}

	if (HitStopRemaining > 0.f)
	{
		// Global time dilation also scales ActorComponent DeltaTime, so use
		// real frame time here. Otherwise a 0.14 s impact freeze at 0.12x
		// dilation would incorrectly last more than a second in real time.
		const float RealDeltaTime = FMath::Max(static_cast<float>(FApp::GetDeltaTime()), 0.f);
		HitStopRemaining -= RealDeltaTime;
		if (HitStopRemaining <= 0.f && bHitStopApplied)
		{
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), PreviousGlobalTimeDilation);
			bHitStopApplied = false;
		}
	}
}

void UHellwakeCameraDirector::TriggerShake(float Magnitude)
{
	ShakeLevel = FMath::Max(ShakeLevel, Magnitude);
}

void UHellwakeCameraDirector::TriggerHitStop(float Seconds)
{
	if (Seconds <= 0.f || !GetWorld())
	{
		return;
	}

	HitStopRemaining = FMath::Max(HitStopRemaining, Seconds);
	if (!bHitStopApplied)
	{
		PreviousGlobalTimeDilation = UGameplayStatics::GetGlobalTimeDilation(GetWorld());
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), HitStopTimeDilation);
		bHitStopApplied = true;
	}
}
