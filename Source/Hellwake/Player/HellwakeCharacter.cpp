#include "Player/HellwakeCharacter.h"
#include "AbilitySystemComponent.h"
#include "Attributes/HellwakeAttributeSet.h"
#include "Abilities/HellwakeGameplayAbility.h"
#include "Abilities/HellwakeAbility_LightAttack.h"
#include "Abilities/HellwakeAbility_HeavyAttack.h"
#include "Abilities/HellwakeAbility_Dodge.h"
#include "Abilities/HellwakeAbility_Emberbrand.h"
#include "Abilities/HellwakeAbility_Bulwark.h"
#include "Abilities/HellwakeAbility_Ruinfall.h"
#include "Abilities/HellwakeAbility_WakeOfHell.h"
#include "GameplayEffects/HellwakeGE_PlayerDefaults.h"
#include "GameplayEffects/HellwakeGE_PassiveRegen.h"
#include "Combat/HellwakeCombatComponent.h"
#include "Combat/HellwakeVitalityComponent.h"
#include "Camera/HellwakeCameraDirector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "HellwakeGameplayTags.h"
#include "Hellwake.h"

AHellwakeCharacter::AHellwakeCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Full);

	AttributeSet = CreateDefaultSubobject<UHellwakeAttributeSet>(TEXT("AttributeSet"));
	CombatComponent = CreateDefaultSubobject<UHellwakeCombatComponent>(TEXT("CombatComponent"));
	VitalityComponent = CreateDefaultSubobject<UHellwakeVitalityComponent>(TEXT("VitalityComponent"));

	PlaceholderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderMesh"));
	PlaceholderMesh->SetupAttachment(RootComponent);
	PlaceholderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlaceholderMesh->SetRelativeScale3D(FVector(0.62f, 0.62f, 1.75f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlayerMeshFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (PlayerMeshFinder.Succeeded())
	{
		PlaceholderMesh->SetStaticMesh(PlayerMeshFinder.Object);
	}

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 620.f, 0.f);
	GetCharacterMovement()->MaxAcceleration = 6200.f;
	GetCharacterMovement()->MaxWalkSpeed = 940.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 4000.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 3700.f;
	CameraBoom->SetRelativeRotation(FRotator(-47.f, -45.f, 0.f));
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 6.f;
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;

	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCamera->FieldOfView = 34.f;
	TopDownCamera->bUsePawnControlRotation = false;

	CameraDirector = CreateDefaultSubobject<UHellwakeCameraDirector>(TEXT("CameraDirector"));

	DefaultAttributesEffectClass = UHellwakeGE_PlayerDefaults::StaticClass();
	PassiveRegenEffectClass = UHellwakeGE_PassiveRegen::StaticClass();
	StartingAbilities = {
		UHellwakeAbility_LightAttack::StaticClass(),
		UHellwakeAbility_HeavyAttack::StaticClass(),
		UHellwakeAbility_Dodge::StaticClass(),
		UHellwakeAbility_Emberbrand::StaticClass(),
		UHellwakeAbility_Bulwark::StaticClass(),
		UHellwakeAbility_Ruinfall::StaticClass(),
		UHellwakeAbility_WakeOfHell::StaticClass()
	};
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
}

UAbilitySystemComponent* AHellwakeCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AHellwakeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EIC && MoveAction)
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHellwakeCharacter::HandleMove);
	}
	else
	{
		PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AHellwakeCharacter::HandleMoveForward);
		PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AHellwakeCharacter::HandleMoveRight);
	}

	auto BindEnhancedAbility = [this, EIC](UInputAction* Action, FGameplayTag Tag)
	{
		if (EIC && Action)
		{
			EIC->BindAction(Action, ETriggerEvent::Started, this, &AHellwakeCharacter::ActivateAbilityByTag, Tag);
			return true;
		}
		return false;
	};

	if (!BindEnhancedAbility(LightAttackAction, HellwakeTags::Ability_Attack_Light))
		PlayerInputComponent->BindAction(TEXT("LightAttack"), IE_Pressed, this, &AHellwakeCharacter::ActivateLightAttack);
	if (!BindEnhancedAbility(HeavyAttackAction, HellwakeTags::Ability_Attack_Heavy))
		PlayerInputComponent->BindAction(TEXT("HeavyAttack"), IE_Pressed, this, &AHellwakeCharacter::ActivateHeavyAttack);
	if (!BindEnhancedAbility(DodgeAction, HellwakeTags::Ability_Dodge))
		PlayerInputComponent->BindAction(TEXT("Dodge"), IE_Pressed, this, &AHellwakeCharacter::ActivateDodge);
	if (!BindEnhancedAbility(EmberbrandAction, HellwakeTags::Ability_Emberbrand))
		PlayerInputComponent->BindAction(TEXT("Emberbrand"), IE_Pressed, this, &AHellwakeCharacter::ActivateEmberbrand);
	if (!BindEnhancedAbility(BulwarkAction, HellwakeTags::Ability_Bulwark))
		PlayerInputComponent->BindAction(TEXT("Bulwark"), IE_Pressed, this, &AHellwakeCharacter::ActivateBulwark);
	if (!BindEnhancedAbility(RuinfallAction, HellwakeTags::Ability_Ruinfall))
		PlayerInputComponent->BindAction(TEXT("Ruinfall"), IE_Pressed, this, &AHellwakeCharacter::ActivateRuinfall);
	if (!BindEnhancedAbility(WakeOfHellAction, HellwakeTags::Ability_WakeOfHell))
		PlayerInputComponent->BindAction(TEXT("WakeOfHell"), IE_Pressed, this, &AHellwakeCharacter::ActivateWakeOfHell);
}

void AHellwakeCharacter::HandleMove(const FInputActionValue& Value)
{
	if (!AttributeSet || !AbilitySystemComponent)
	{
		return;
	}
	if (AbilitySystemComponent->HasMatchingGameplayTag(HellwakeTags::State_Dead) ||
		AbilitySystemComponent->HasMatchingGameplayTag(HellwakeTags::State_Cinematic))
	{
		return;
	}

	const FVector2D Axis = Value.Get<FVector2D>();
	const FVector Direction = FVector(Axis.X, -Axis.Y, 0.f);
	if (!Direction.IsNearlyZero())
	{
		AddMovementInput(Direction, 1.f);
	}
}

void AHellwakeCharacter::HandleMoveForward(float Value)
{
	if (FMath::IsNearlyZero(Value) || !AbilitySystemComponent)
	{
		return;
	}
	if (AbilitySystemComponent->HasMatchingGameplayTag(HellwakeTags::State_Dead) ||
		AbilitySystemComponent->HasMatchingGameplayTag(HellwakeTags::State_Cinematic))
	{
		return;
	}
	AddMovementInput(FVector(0.f, -1.f, 0.f), Value);
}

void AHellwakeCharacter::HandleMoveRight(float Value)
{
	if (FMath::IsNearlyZero(Value) || !AbilitySystemComponent)
	{
		return;
	}
	if (AbilitySystemComponent->HasMatchingGameplayTag(HellwakeTags::State_Dead) ||
		AbilitySystemComponent->HasMatchingGameplayTag(HellwakeTags::State_Cinematic))
	{
		return;
	}
	AddMovementInput(FVector(1.f, 0.f, 0.f), Value);
}

void AHellwakeCharacter::ActivateAbilityByTag(FGameplayTag Tag)
{
	if (AbilitySystemComponent)
	{
		FGameplayTagContainer Container(Tag);
		AbilitySystemComponent->TryActivateAbilitiesByTag(Container);
	}
}

void AHellwakeCharacter::ActivateLightAttack() { ActivateAbilityByTag(HellwakeTags::Ability_Attack_Light); }
void AHellwakeCharacter::ActivateHeavyAttack() { ActivateAbilityByTag(HellwakeTags::Ability_Attack_Heavy); }
void AHellwakeCharacter::ActivateDodge() { ActivateAbilityByTag(HellwakeTags::Ability_Dodge); }
void AHellwakeCharacter::ActivateEmberbrand() { ActivateAbilityByTag(HellwakeTags::Ability_Emberbrand); }
void AHellwakeCharacter::ActivateBulwark() { ActivateAbilityByTag(HellwakeTags::Ability_Bulwark); }
void AHellwakeCharacter::ActivateRuinfall() { ActivateAbilityByTag(HellwakeTags::Ability_Ruinfall); }
void AHellwakeCharacter::ActivateWakeOfHell() { ActivateAbilityByTag(HellwakeTags::Ability_WakeOfHell); }

void AHellwakeCharacter::OnDeathEvent(const FGameplayEventData* Payload)
{
	Die();
}

void AHellwakeCharacter::Die()
{
	if (!AbilitySystemComponent || AbilitySystemComponent->HasMatchingGameplayTag(HellwakeTags::State_Dead))
	{
		return;
	}

	AbilitySystemComponent->AddLooseGameplayTag(HellwakeTags::State_Dead);
	AbilitySystemComponent->CancelAbilities();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}
	SetActorEnableCollision(false);

	DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(0.f, 0.f, 90.f), 180.f, 18, FColor::Red, false, 0.55f, 0, 8.f);
	if (CameraDirector)
	{
		CameraDirector->TriggerShake(0.85f);
	}

	GetWorld()->GetTimerManager().SetTimer(RespawnHandle, [this]()
	{
		if (!AbilitySystemComponent || !AttributeSet)
		{
			return;
		}

		SetActorLocation(SpawnPoint, false, nullptr, ETeleportType::TeleportPhysics);
		SetActorEnableCollision(true);
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			Movement->StopMovementImmediately();
			Movement->SetMovementMode(MOVE_Walking);
		}

		AbilitySystemComponent->ApplyModToAttribute(UHellwakeAttributeSet::GetHealthAttribute(), EGameplayModOp::Override, AttributeSet->GetMaxHealth());
		AbilitySystemComponent->ApplyModToAttribute(UHellwakeAttributeSet::GetWrathAttribute(), EGameplayModOp::Override, AttributeSet->GetMaxWrath());
		AbilitySystemComponent->RemoveLooseGameplayTag(HellwakeTags::State_Dead);

		DrawDebugSphere(GetWorld(), SpawnPoint + FVector(0.f, 0.f, 90.f), 140.f, 16, FColor::Cyan, false, 0.4f, 0, 6.f);
	}, RespawnDelaySeconds, false);
}

void AHellwakeCharacter::BeginCinematic(float Duration)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->AddLooseGameplayTag(HellwakeTags::State_Cinematic);
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}
	if (CameraDirector)
	{
		CameraDirector->SetCinematicActive(true);
	}
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
	if (CameraDirector)
	{
		CameraDirector->SetCinematicActive(false);
	}
}

void AHellwakeCharacter::SetBossFightCameraActive(bool bActive)
{
	if (CameraDirector)
	{
		CameraDirector->SetBossFightActive(bActive);
	}
}
