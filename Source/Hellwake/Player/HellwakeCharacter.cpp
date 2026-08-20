#include "Player/HellwakeCharacter.h"
#include "AbilitySystemComponent.h"
#include "Attributes/HellwakeAttributeSet.h"
#include "Abilities/HellwakeGameplayAbility.h"
#include "Combat/HellwakeCombatComponent.h"
#include "Combat/HellwakeVitalityComponent.h"
#include "Camera/HellwakeCameraDirector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "HellwakeGameplayTags.h"
#include "Hellwake.h"

AHellwakeCharacter::AHellwakeCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UHellwakeAttributeSet>(TEXT("AttributeSet"));

	CombatComponent = CreateDefaultSubobject<UHellwakeCombatComponent>(TEXT("CombatComponent"));
	VitalityComponent = CreateDefaultSubobject<UHellwakeVitalityComponent>(TEXT("VitalityComponent"));

	// Facing follows movement direction only — no controller yaw binding.
	// Prototype: `hero.rotation.y = P.facing` derived purely from velocity.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// Prototype accel 62 / speed 9.4 -> time-to-max-speed ~0.15s; RotationRate
	// 14*dt lerp on facing translates to a fast, snappy turn rate.
	GetCharacterMovement()->RotationRate = FRotator(0.f, 620.f, 0.f);
	GetCharacterMovement()->MaxAcceleration = 6200.f;   // 62 u/s^2 * 100
	GetCharacterMovement()->MaxWalkSpeed = 940.f;        // 9.4 u/s * 100
	GetCharacterMovement()->BrakingDecelerationWalking = 4000.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	// Prototype CAM_OFF = (0, 27, 25); camera.lookAt(hero.xz, y=2.4, z-4.6).
	// A fixed isometric boom with no arm-length/rotation input reproduces
	// this: rotate to look down-and-forward at the approved ~34deg FOV
	// framing, do not let player input rotate it.
	CameraBoom->TargetArmLength = 3700.f;
	CameraBoom->SetRelativeRotation(FRotator(-47.f, -45.f, 0.f));
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 6.f; // prototype: camTarget.lerp(focus, min(1,6*dt))
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;

	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCamera->FieldOfView = 34.f;
	TopDownCamera->bUsePawnControlRotation = false;

	CameraDirector = CreateDefaultSubobject<UHellwakeCameraDirector>(TEXT("CameraDirector"));
}

void AHellwakeCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);

		if (DefaultAttributesEffectClass)
		{
			FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
			FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(DefaultAttributesEffectClass, 1.f, Context);
			if (Spec.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data);
			}
		}

		for (const TSubclassOf<UHellwakeGameplayAbility>& AbilityClass : StartingAbilities)
		{
			if (AbilityClass)
			{
				AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
			}
		}

		if (PassiveRegenEffectClass)
		{
			FGameplayEffectContextHandle RegenContext = AbilitySystemComponent->MakeEffectContext();
			FGameplayEffectSpecHandle RegenSpec = AbilitySystemComponent->MakeOutgoingSpec(PassiveRegenEffectClass, 1.f, RegenContext);
			if (RegenSpec.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*RegenSpec.Data);
			}
		}

		AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(HellwakeTags::Event_Death)
			.AddUObject(this, &AHellwakeCharacter::OnDeathEvent);
	}

	if (APlayerController* PC = Cast<APlayerController>(NewController))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void AHellwakeCharacter::BeginPlay()
{
	Super::BeginPlay();
	SetActorLocation(SpawnPoint);
}

void AHellwakeCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// Passive regen (Wrath 4.5/s, Health 1.6/s while alive and not full) is
	// implemented as an infinite periodic GameplayEffect
	// (GE_Hellwake_PassiveRegen, applied once in PossessedBy) rather than
	// here, so it keeps working under GAS prediction/replication. Nothing
	// prototype-specific belongs in TickActor for this character; CombatComponent
	// owns per-frame facing/i-frame bookkeeping that isn't already covered
	// by CharacterMovementComponent + GameplayEffect durations.
}

UAbilitySystemComponent* AHellwakeCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AHellwakeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		UE_LOG(LogHellwake, Error, TEXT("HellwakeCharacter expects an Enhanced Input component."));
		return;
	}

	if (MoveAction)
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHellwakeCharacter::HandleMove);
	}
	// LMB/RMB/Shift/Q/F/E/R all just resolve to "try activate ability with
	// this tag" — CanActivateAbility (cost/cooldown/blocked-tags) does the
	// rest, matching the prototype's uniform `if (cds[key] > 0) return;`
	// gate at the top of every input handler.
	auto BindAbility = [this, EIC](UInputAction* Action, FGameplayTag Tag)
	{
		if (Action)
		{
			EIC->BindAction(Action, ETriggerEvent::Started, this, &AHellwakeCharacter::ActivateAbilityByTag, Tag);
		}
	};
	BindAbility(LightAttackAction, HellwakeTags::Ability_Attack_Light);
	BindAbility(HeavyAttackAction, HellwakeTags::Ability_Attack_Heavy);
	BindAbility(DodgeAction, HellwakeTags::Ability_Dodge);
	BindAbility(EmberbrandAction, HellwakeTags::Ability_Emberbrand);
	BindAbility(BulwarkAction, HellwakeTags::Ability_Bulwark);
	BindAbility(RuinfallAction, HellwakeTags::Ability_Ruinfall);
	BindAbility(WakeOfHellAction, HellwakeTags::Ability_WakeOfHell);
}

void AHellwakeCharacter::HandleMove(const FInputActionValue& Value)
{
	if (GetHellwakeAttributeSet() == nullptr || AbilitySystemComponent == nullptr)
	{
		return;
	}
	if (AbilitySystemComponent->HasMatchingGameplayTag(HellwakeTags::State_Dead) ||
		AbilitySystemComponent->HasMatchingGameplayTag(HellwakeTags::State_Cinematic))
	{
		return;
	}

	const FVector2D Axis = Value.Get<FVector2D>();
	// World-axis movement, not camera-relative-rotated: prototype moveDir()
	// maps w/s/a/d straight onto +-Z/+-X with no rotation by camera yaw
	// (the camera never yaws), so this is already correct as-is.
	const FVector Direction = FVector(Axis.X, -Axis.Y, 0.f);
	if (!Direction.IsNearlyZero())
	{
		AddMovementInput(Direction, 1.f);
	}
}

void AHellwakeCharacter::ActivateAbilityByTag(FGameplayTag Tag)
{
	if (AbilitySystemComponent)
	{
		FGameplayTagContainer Container(Tag);
		AbilitySystemComponent->TryActivateAbilitiesByTag(Container);
	}
}

void AHellwakeCharacter::OnDeathEvent(const FGameplayEventData* Payload)
{
	Die();
}

void AHellwakeCharacter::Die()
{
	if (!AbilitySystemComponent)
	{
		return;
	}
	AbilitySystemComponent->AddLooseGameplayTag(HellwakeTags::State_Dead);
	AbilitySystemComponent->CancelAbilities();

	// TODO(Presentation): banner('YOU FELL — RESPAWNING') -> broadcast to HUD.
	GetWorld()->GetTimerManager().SetTimer(RespawnHandle, [this]()
	{
		if (!AbilitySystemComponent || !AttributeSet)
		{
			return;
		}
		AbilitySystemComponent->RemoveLooseGameplayTag(HellwakeTags::State_Dead);
		AbilitySystemComponent->ApplyModToAttribute(UHellwakeAttributeSet::GetHealthAttribute(), EGameplayModOp::Override, AttributeSet->GetMaxHealth());
		AbilitySystemComponent->ApplyModToAttribute(UHellwakeAttributeSet::GetWrathAttribute(), EGameplayModOp::Override, AttributeSet->GetMaxWrath());
		SetActorLocation(SpawnPoint);
	}, RespawnDelaySeconds, false);
}

void AHellwakeCharacter::BeginCinematic(float Duration)
{
	if (!AbilitySystemComponent)
	{
		return;
	}
	AbilitySystemComponent->AddLooseGameplayTag(HellwakeTags::State_Cinematic);
	if (Duration > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(CinematicEndHandle, this, &AHellwakeCharacter::EndCinematic, Duration, false);
	}
}

void AHellwakeCharacter::EndCinematic()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(HellwakeTags::State_Cinematic);
	}
}
