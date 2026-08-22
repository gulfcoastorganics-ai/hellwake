// Asset-free fallback HUD used by the first native PIE milestone. The final
// presentation HUD remains WBP_HellwakeHUD/UHellwakeHUDWidget, but a clean C++
// checkout should still expose the core combat/encounter state before any UMG
// assets exist.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HellwakeDebugHUD.generated.h"

UCLASS()
class HELLWAKE_API AHellwakeDebugHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawMeter(const FString& Label, float Current, float Max, float X, float Y, float Width, const FLinearColor& FillColor);
};
