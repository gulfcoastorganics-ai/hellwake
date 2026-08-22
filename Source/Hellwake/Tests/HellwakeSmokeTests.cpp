#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Attributes/HellwakeAttributeSet.h"
#include "Enemies/HellwakeEnemyBase.h"
#include "Enemies/HellwakeGravewarden.h"
#include "HellwakeGameplayTags.h"
#include "HellwakeGameMode.h"
#include "HUD/HellwakeDebugHUD.h"
#include "Loot/HellwakeLootPickup.h"
#include "Player/HellwakeCharacter.h"
#include "Player/HellwakePlayerController.h"
#include "GameplayEffects/HellwakeGE_AbilityRuntime.h"
#include "GameplayEffects/HellwakeGE_Damage.h"
#include "GameplayEffects/HellwakeGE_PlayerDefaults.h"
#include "GameplayEffects/HellwakeGE_PassiveRegen.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHellwakeNativeTagsSmokeTest,
	"Hellwake.Smoke.NativeGameplayTags",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHellwakeNativeTagsSmokeTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Light attack tag registered"), HellwakeTags::Ability_Attack_Light.IsValid());
	TestTrue(TEXT("Heavy attack tag registered"), HellwakeTags::Ability_Attack_Heavy.IsValid());
	TestTrue(TEXT("Dodge tag registered"), HellwakeTags::Ability_Dodge.IsValid());
	TestTrue(TEXT("Emberbrand tag registered"), HellwakeTags::Ability_Emberbrand.IsValid());
	TestTrue(TEXT("Bulwark tag registered"), HellwakeTags::Ability_Bulwark.IsValid());
	TestTrue(TEXT("Ruinfall tag registered"), HellwakeTags::Ability_Ruinfall.IsValid());
	TestTrue(TEXT("Wake of Hell tag registered"), HellwakeTags::Ability_WakeOfHell.IsValid());

	TestTrue(TEXT("Light cooldown tag registered"), HellwakeTags::Cooldown_LightAttack.IsValid());
	TestTrue(TEXT("Heavy cooldown tag registered"), HellwakeTags::Cooldown_HeavyAttack.IsValid());
	TestTrue(TEXT("Dodge cooldown tag registered"), HellwakeTags::Cooldown_Dodge.IsValid());
	TestTrue(TEXT("Emberbrand cooldown tag registered"), HellwakeTags::Cooldown_Emberbrand.IsValid());
	TestTrue(TEXT("Bulwark cooldown tag registered"), HellwakeTags::Cooldown_Bulwark.IsValid());
	TestTrue(TEXT("Ruinfall cooldown tag registered"), HellwakeTags::Cooldown_Ruinfall.IsValid());
	TestTrue(TEXT("Wake of Hell cooldown tag registered"), HellwakeTags::Cooldown_WakeOfHell.IsValid());

	TestTrue(TEXT("Death tag registered"), HellwakeTags::Event_Death.IsValid());
	TestTrue(TEXT("Damage SetByCaller tag registered from config"), FGameplayTag::RequestGameplayTag(FName(TEXT("Data.Damage")), false).IsValid());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHellwakeAttributeDefaultsSmokeTest,
	"Hellwake.Smoke.AttributeDefaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHellwakeAttributeDefaultsSmokeTest::RunTest(const FString& Parameters)
{
	const UHellwakeAttributeSet* Attributes = NewObject<UHellwakeAttributeSet>();
	TestNotNull(TEXT("AttributeSet can be constructed"), Attributes);
	if (!Attributes)
	{
		return false;
	}

	TestEqual(TEXT("Health default"), Attributes->GetHealth(), 100.f);
	TestEqual(TEXT("MaxHealth default"), Attributes->GetMaxHealth(), 100.f);
	TestEqual(TEXT("Wrath default"), Attributes->GetWrath(), 100.f);
	TestEqual(TEXT("MaxWrath default"), Attributes->GetMaxWrath(), 100.f);
	TestEqual(TEXT("Health regen default"), Attributes->GetHealthRegenRate(), 1.6f);
	TestEqual(TEXT("Wrath regen default"), Attributes->GetWrathRegenRate(), 4.5f);
	TestEqual(TEXT("Incoming damage multiplier default"), Attributes->GetIncomingDamageMultiplier(), 1.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FHellwakeNativeFrameworkSmokeTest,
	"Hellwake.Smoke.NativeFrameworkWiring",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FHellwakeNativeFrameworkSmokeTest::RunTest(const FString& Parameters)
{
	const AHellwakeGameMode* GameModeCDO = GetDefault<AHellwakeGameMode>();
	TestNotNull(TEXT("Hellwake GameMode CDO exists"), GameModeCDO);
	if (!GameModeCDO)
	{
		return false;
	}

	TestEqual(TEXT("Native player pawn is default"), GameModeCDO->DefaultPawnClass.Get(), AHellwakeCharacter::StaticClass());
	TestEqual(TEXT("Native player controller is default"), GameModeCDO->PlayerControllerClass.Get(), AHellwakePlayerController::StaticClass());
	TestEqual(TEXT("Native debug HUD is default"), GameModeCDO->HUDClass.Get(), AHellwakeDebugHUD::StaticClass());

	TestNotNull(TEXT("Native damage GameplayEffect exists"), GetDefault<UHellwakeGE_Damage>());
	TestNotNull(TEXT("Native player-defaults GameplayEffect exists"), GetDefault<UHellwakeGE_PlayerDefaults>());
	TestNotNull(TEXT("Native passive-regen GameplayEffect exists"), GetDefault<UHellwakeGE_PassiveRegen>());
	TestNotNull(TEXT("Native Emberbrand cooldown exists"), GetDefault<UHellwakeGE_Cooldown_Emberbrand>());
	TestNotNull(TEXT("Native Bulwark cost exists"), GetDefault<UHellwakeGE_Cost_Bulwark>());
	TestNotNull(TEXT("Native Ruinfall cost exists"), GetDefault<UHellwakeGE_Cost_Ruinfall>());

	TestNotNull(TEXT("Native enemy base class exists"), GetDefault<AHellwakeEnemyBase>());
	TestNotNull(TEXT("Native Gravewarden class exists"), GetDefault<AHellwakeGravewarden>());
	TestNotNull(TEXT("Native loot pickup class exists"), GetDefault<AHellwakeLootPickup>());
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
