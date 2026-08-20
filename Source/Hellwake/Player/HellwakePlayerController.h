// Minimal — most setup lives on HellwakeCharacter (ASC owner, input
// bindings). This class exists for the HUD widget lifecycle and any
// controller-level concerns (pause, cursor) that don't belong on the pawn.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HellwakePlayerController.generated.h"

class UHellwakeHUDWidget;

UCLASS()
class HELLWAKE_API AHellwakePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AHellwakePlayerController();

protected:
	virtual void BeginPlay() override;

	// UMG root widget class — see project-bible/hud.md for the full field
	// list this widget must bind to (mirrors the .dc.html HUD template's
	// {{ }} interpolations 1:1).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|HUD")
	TSubclassOf<UHellwakeHUDWidget> HUDWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Hellwake|HUD")
	TObjectPtr<UHellwakeHUDWidget> HUDWidget;
};
