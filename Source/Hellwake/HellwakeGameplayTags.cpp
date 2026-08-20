#include "HellwakeGameplayTags.h"

namespace HellwakeTags
{
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack_Light, "Ability.Attack.Light");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack_Heavy, "Ability.Attack.Heavy");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Dodge, "Ability.Dodge");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Emberbrand, "Ability.Emberbrand");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Bulwark, "Ability.Bulwark");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Ruinfall, "Ability.Ruinfall");
	UE_DEFINE_GAMEPLAY_TAG(Ability_WakeOfHell, "Ability.WakeOfHell");

	UE_DEFINE_GAMEPLAY_TAG(Boss_GravewardenPhase_SweepingAxe, "Boss.Gravewarden.Attack.SweepingAxe");
	UE_DEFINE_GAMEPLAY_TAG(Boss_GravewardenPhase_GroundSlam, "Boss.Gravewarden.Attack.GroundSlam");
	UE_DEFINE_GAMEPLAY_TAG(Boss_GravewardenPhase_AreaDenial, "Boss.Gravewarden.Attack.AreaDenial");

	UE_DEFINE_GAMEPLAY_TAG(State_Dead, "State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Cinematic, "State.Cinematic");
	UE_DEFINE_GAMEPLAY_TAG(State_IFrame, "State.IFrame");
	UE_DEFINE_GAMEPLAY_TAG(State_Staggered, "State.Staggered");
	UE_DEFINE_GAMEPLAY_TAG(State_AbilityCooldown, "State.AbilityCooldown");
	UE_DEFINE_GAMEPLAY_TAG(State_AttackCone_Light, "State.AttackCone.Light");
	UE_DEFINE_GAMEPLAY_TAG(State_AttackCone_Heavy, "State.AttackCone.Heavy");

	UE_DEFINE_GAMEPLAY_TAG(Status_Ember, "Status.Ember");
	UE_DEFINE_GAMEPLAY_TAG(Status_Fortified, "Status.Fortified");
	UE_DEFINE_GAMEPLAY_TAG(Status_SoulrendAura, "Status.SoulrendAura");

	UE_DEFINE_GAMEPLAY_TAG(Event_Damage_Taken, "Event.Damage.Taken");
	UE_DEFINE_GAMEPLAY_TAG(Event_Damage_Dealt, "Event.Damage.Dealt");
	UE_DEFINE_GAMEPLAY_TAG(Event_Damage_Crit, "Event.Damage.Crit");
	UE_DEFINE_GAMEPLAY_TAG(Event_Death, "Event.Death");
	UE_DEFINE_GAMEPLAY_TAG(Event_Loot_Dropped, "Event.Loot.Dropped");
	UE_DEFINE_GAMEPLAY_TAG(Event_Loot_PickedUp, "Event.Loot.PickedUp");
	UE_DEFINE_GAMEPLAY_TAG(Event_Encounter_StageChanged, "Event.Encounter.StageChanged");

	UE_DEFINE_GAMEPLAY_TAG(Enemy_Role_Reaver, "Enemy.Role.Reaver");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Role_Wraith, "Enemy.Role.Wraith");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Role_Acolyte, "Enemy.Role.Acolyte");
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Role_Gravewarden, "Enemy.Role.Gravewarden");
}
