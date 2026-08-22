// Native gameplay effects that make the seven player abilities fully usable
// in a clean C++ checkout: cooldowns, Wrath costs, and Emberbrand's cosmetic
// status tag. Blueprint children can still replace these later for tuning.
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffects/HellwakeGE_Cooldown.h"
#include "HellwakeGE_AbilityRuntime.generated.h"

UCLASS()
class HELLWAKE_API UHellwakeGE_Cooldown_LightAttack : public UHellwakeGE_Cooldown
{
	GENERATED_BODY()
public:
	UHellwakeGE_Cooldown_LightAttack();
};

UCLASS()
class HELLWAKE_API UHellwakeGE_Cooldown_HeavyAttack : public UHellwakeGE_Cooldown
{
	GENERATED_BODY()
public:
	UHellwakeGE_Cooldown_HeavyAttack();
};

UCLASS()
class HELLWAKE_API UHellwakeGE_Cooldown_Dodge : public UHellwakeGE_Cooldown
{
	GENERATED_BODY()
public:
	UHellwakeGE_Cooldown_Dodge();
};

UCLASS()
class HELLWAKE_API UHellwakeGE_Cooldown_Emberbrand : public UHellwakeGE_Cooldown
{
	GENERATED_BODY()
public:
	UHellwakeGE_Cooldown_Emberbrand();
};

UCLASS()
class HELLWAKE_API UHellwakeGE_Cooldown_Bulwark : public UHellwakeGE_Cooldown
{
	GENERATED_BODY()
public:
	UHellwakeGE_Cooldown_Bulwark();
};

UCLASS()
class HELLWAKE_API UHellwakeGE_Cooldown_Ruinfall : public UHellwakeGE_Cooldown
{
	GENERATED_BODY()
public:
	UHellwakeGE_Cooldown_Ruinfall();
};

UCLASS()
class HELLWAKE_API UHellwakeGE_Cooldown_WakeOfHell : public UHellwakeGE_Cooldown
{
	GENERATED_BODY()
public:
	UHellwakeGE_Cooldown_WakeOfHell();
};

UCLASS()
class HELLWAKE_API UHellwakeGE_Cost_Emberbrand : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UHellwakeGE_Cost_Emberbrand();
};

UCLASS()
class HELLWAKE_API UHellwakeGE_Cost_Bulwark : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UHellwakeGE_Cost_Bulwark();
};

UCLASS()
class HELLWAKE_API UHellwakeGE_Cost_Ruinfall : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UHellwakeGE_Cost_Ruinfall();
};

UCLASS()
class HELLWAKE_API UHellwakeGE_Status_Ember : public UGameplayEffect
{
	GENERATED_BODY()
public:
	UHellwakeGE_Status_Ember();
};
