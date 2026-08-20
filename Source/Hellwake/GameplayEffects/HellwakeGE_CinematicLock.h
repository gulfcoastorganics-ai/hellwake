// Duration effect. Grants State.Cinematic, which every
// UHellwakeGameplayAbility already blocks activation on. Duration is set
// per-use case in-editor Blueprint children (Wake of Hell: 1.6s; boss
// intro/death: ~3.2-3.4s per docs/hellwake-behavioral-spec.md — the base
// class defaults to Wake of Hell's value since that's the only ability
// that self-applies this GE; encounter-driven cinematics are instead
// started/ended explicitly via HellwakeCharacter::BeginCinematic/EndCinematic).
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "HellwakeGE_CinematicLock.generated.h"

UCLASS()
class HELLWAKE_API UHellwakeGE_CinematicLock : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UHellwakeGE_CinematicLock();
};
