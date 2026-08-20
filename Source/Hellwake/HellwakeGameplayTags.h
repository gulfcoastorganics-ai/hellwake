// Central native GameplayTag declarations for Hellwake.
// Mirrors Config/Tags/GameplayTags.ini — keep both in sync. Native tags here
// are what C++ code should reference directly instead of FName string
// lookups; the .ini registers the same tags for Blueprint/data-table use.
#pragma once

#include "NativeGameplayTags.h"

namespace HellwakeTags
{
	// Ability identity tags — one per player/boss ability, used both to
	// activate abilities by tag and to gate them (e.g. "can't dodge while
	// dead", "can't attack during cinematic").
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Light);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Attack_Heavy);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Dodge);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Emberbrand);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Bulwark);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Ruinfall);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_WakeOfHell);

	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Boss_GravewardenPhase_SweepingAxe);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Boss_GravewardenPhase_GroundSlam);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Boss_GravewardenPhase_AreaDenial);

	// State tags — drive UI, ability gating, and animation state.
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Dead);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Cinematic);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_IFrame);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Staggered);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_AbilityCooldown);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_AttackCone_Light);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_AttackCone_Heavy);

	// Status effect tags — mirrors the prototype's HUD status pips.
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Ember);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_Fortified);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_SoulrendAura);

	// Damage / event tags used by GameplayEffect execution calculations and
	// gameplay cue triggers.
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Damage_Taken);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Damage_Dealt);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Damage_Crit);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Death);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Loot_Dropped);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Loot_PickedUp);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Encounter_StageChanged);

	// Enemy role tags — used by the State Tree AI and by encounter spawn
	// tables to select behavior without a C++ switch on an enum.
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Role_Reaver);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Role_Wraith);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Role_Acolyte);
	HELLWAKE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Role_Gravewarden);
}
