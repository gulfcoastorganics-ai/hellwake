#include "Player/HellwakePlayerController.h"
#include "HUD/HellwakeHUDWidget.h"

AHellwakePlayerController::AHellwakePlayerController()
{
}

void AHellwakePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() && HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UHellwakeHUDWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
			HUDWidget->BindToPlayerController(this);
		}
	}
}
