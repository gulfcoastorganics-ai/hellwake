// Kaervoss, the player character. Owns its own ASC (player-state-less
// single-player-first design — see project-bible/unreal-implementation.md
// for the multiplayer note on moving the ASC to PlayerState later).
#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "HellwakeCharacter.generated.h"

class UAbilitySystemComponent;
class UHellwakeAttributeSet;
class UHellwakeCombatComponent;
class UHellwakeVitalityComponent;
class UHellwakeCameraDirector;
class UHellwakeGameplayAbility;
class USpringArmComponent;
class UCameraComponent;
class UStaticMeshComponent;
class UGameplayEffect;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
struct FGameplayEventData;

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

	void Die();

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
	TObjectPtr<UHellwakeVitalityComponent> VitalityComponent;

	// Engine-basic-shape fallback so Kaervoss is visible in a clean native
	// checkout. A skeletal mesh Blueprint can hide/replace this component.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Fallback")
	TObjectPtr<UStaticMeshComponent> PlaceholderMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Camera")
	TObjectPtr<UCameraComponent> TopDownCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hellwake|Camera")
	TObjectPtr<UHellwakeCameraDirector> CameraDirector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Abilities")
	TArray<TSubclassOf<UHellwakeGameplayAbility>> StartingAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hellwake|Abilities")
	TSubclassOf<UGameplayEffect> DefaultAttributesEffectClass;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake|Respawn")
	FVector SpawnPoint = FVector(0.f, 1800.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hellwake|Respawn")
	float RespawnDelaySeconds = 1.8f;

private:
	void HandleMove(const FInputActionValue& Value);
	void HandleMoveForward(float Value);
	void HandleMoveRight(float Value);
	void ActivateAbilityByTag(FGameplayTag Tag);
	void ActivateLightAttack();
	void ActivateHeavyAttack();
	void ActivateDodge();
	void ActivateEmberbrand();
	void ActivateBulwark();
	void ActivateRuinfall();
	void ActivateWakeOfHell();
	void OnDeathEvent(const FGameplayEventData* Payload);

	FTimerHandle RespawnHandle;
	FTimerHandle CinematicEndHandle;
};
