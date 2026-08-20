// Per-frame combat bookkeeping that doesn't fit cleanly into a
// GameplayEffect duration: nothing in the prototype's per-frame combat loop
// actually needs this once GAS owns cooldowns/i-frames/buffs, but this
// component is the intended home for anim-driven hit-window state (the
// "TODO: play a montage, call ResolveMeleeHit from an AnimNotify" note in
// HellwakeAbility_MeleeAttack) once real animations replace the prototype's
// instant-hitscan swings. Attach to the player and to every enemy.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HellwakeCombatComponent.generated.h"

UCLASS(ClassGroup = (Hellwake), meta = (BlueprintSpawnableComponent))
class HELLWAKE_API UHellwakeCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHellwakeCombatComponent();

	// True while a melee-attack AnimMontage's hit window is open. Abilities
	// should gate ResolveMeleeHit() (or an equivalent trace) on this once
	// montages replace the instant hit-scan; kept false/unused (hit
	// resolves on the activation frame) until animation is authored
	// in-editor, per project-bible/combat.md.
	UPROPERTY(BlueprintReadOnly, Category = "Hellwake|Combat")
	bool bHitWindowOpen = false;

	UFUNCTION(BlueprintCallable, Category = "Hellwake|Combat")
	void OpenHitWindow() { bHitWindowOpen = true; }

	UFUNCTION(BlueprintCallable, Category = "Hellwake|Combat")
	void CloseHitWindow() { bHitWindowOpen = false; }
};
