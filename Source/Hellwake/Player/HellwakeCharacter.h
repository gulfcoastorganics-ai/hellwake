// Kaervoss, the player character. Owns its own ASC (player-state-less
// single-player-first design — see project-bible/unreal-implementation.md
// for the multiplayer note on moving the ASC to PlayerState later).
//
// Movement/facing model ports the prototype exactly: CharacterMovement
// orients rotation to movement input (bOrientRotationToMovement = true), so
// "facing" is purely a function of the last movement direction, matching
// `P.facing += angDiff(atan2(vel.x, vel.z), P.facing) * min(1, 14*dt)` in
// hellwake-game.js. There is deliberately no controller-yaw / mouse-look
// binding here — see project-bible/combat.md for the targeting discussion.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "HellwakeCharacter.generated.h"

class UAbilitySystemComponent;
class UHellwakeAttributeSet;
class UHellwakeCombatComponent;
class USpringArmComponent;
class UCameraComponent;
class UGameplayEffect;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class HELLWAKE_API AHellwakeCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AHellwakeCharacter();

	virtual void PossessedBy(AController* NewController) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintPure, Category = "Hellwake|Combat")
	UHellwakeAttributeSet* GetHellwakeAttributeSet() const { return AttributeSet; }

	// Called from HellwakeAttributeSet::PostGameplayEffectExecute via the
	// Event.Death gameplay event -> bound in BeginPlay. Ports respawn():
	// 1.8s delay, then full HP/Wrath restore + teleport to SpawnPoint.
	void Die();

	// Ports setStage('intro')'s `cine = 3.2` and similar cinematic locks:
	// grants State.Cinematic for Duration seconds (0 = until EndCinematic).
	UFUNCTION(BlueprintCallable, Category = "Hellwake|Cinematic")
	void BeginCinematic(float Duration);
	UFUNCTION(BlueprintCallable, Category = "Hellwake|Cinematic")
	void EndCinematic();

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Abilities")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Abilities")
	TObjectPtr<UHellwakeAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Combat")
	TObjectPtr<UHellwakeCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Combat")
	TObjectPtr<class UHellwakeVitalityComponent> VitalityComponent;

	// Fixed isometric boom — never rotates with input, matching the
	// prototype's static CAM_OFF = (0, 27, 25) camera offset (see
	// HellwakeCameraComponent for the zoom/shake/target-lerp behavior that
	// layers on top of this boom).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Camera")
	TObjectPtr<UCameraComponent> TopDownCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Camera")
	TObjectPtr<class UHellwakeCameraDirector> CameraDirector;

	// Granted on PossessedBy. One entry per HellwakeTags::Ability_* tag;
	// index-matched pairing is intentional (kept simple over a TMap for
	// designer readability in the details panel).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Abilities")
	TArray<TSubclassOf<class UHellwakeGameplayAbility>> StartingAbilities;

	// Default attribute values applied via an instant GameplayEffect on
	// BeginPlay instead of hardcoding into the AttributeSet constructor, so
	// designers can retune without a recompile. GE_Hellwake_PlayerDefaults.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Abilities")
	TSubclassOf<UGameplayEffect> DefaultAttributesEffectClass;

	// GE_Hellwake_PassiveRegen — applied once on possess, runs for the
	// character's lifetime.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Abilities")
	TSubclassOf<UGameplayEffect> PassiveRegenEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Input")
	TObjectPtr<UInputAction> LightAttackAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Input")
	TObjectPtr<UInputAction> HeavyAttackAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Input")
	TObjectPtr<UInputAction> DodgeAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Input")
	TObjectPtr<UInputAction> EmberbrandAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Input")
	TObjectPtr<UInputAction> BulwarkAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Input")
	TObjectPtr<UInputAction> RuinfallAction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Input")
	TObjectPtr<UInputAction> WakeOfHellAction;

	// Prototype: hero.position.set(0,0,18) both at boot and in respawn().
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake|Respawn")
	FVector SpawnPoint = FVector(0.f, 1800.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake|Respawn")
	float RespawnDelaySeconds = 1.8f;

private:
	void HandleMove(const FInputActionValue& Value);
	void ActivateAbilityByTag(FGameplayTag Tag);

	void OnDeathEvent(const FGameplayEventData* Payload);

	FTimerHandle RespawnHandle;
	FTimerHandle CinematicEndHandle;
};
