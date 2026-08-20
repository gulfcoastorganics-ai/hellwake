#include "Camera/HellwakeCameraDirector.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

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
		HitStopRemaining -= DeltaTime;
		if (HitStopRemaining <= 0.f && bHitStopApplied)
		{
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
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
	HitStopRemaining = FMath::Max(HitStopRemaining, Seconds);
	if (!bHitStopApplied)
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), HitStopTimeDilation);
		bHitStopApplied = true;
	}
}
