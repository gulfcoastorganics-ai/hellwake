#include "Enemies/HellwakeEnemyBase.h"
#include "Enemies/HellwakeEnemyAIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Attributes/HellwakeAttributeSet.h"
#include "Combat/HellwakeVitalityComponent.h"
#include "GameplayEffects/HellwakeGE_Damage.h"
#include "Loot/HellwakeLootDropComponent.h"
#include "Data/HellwakeEnemyDefinition.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "HellwakeGameplayTags.h"
#include "Hellwake.h"

AHellwakeEnemyBase::AHellwakeEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UHellwakeAttributeSet>(TEXT("AttributeSet"));
	VitalityComponent = CreateDefaultSubobject<UHellwakeVitalityComponent>(TEXT("VitalityComponent"));
	LootDropComponent = CreateDefaultSubobject<UHellwakeLootDropComponent>(TEXT("LootDropComponent"));
	DamageEffectClass = UHellwakeGE_Damage::StaticClass();

	PlaceholderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderMesh"));
	PlaceholderMesh->SetupAttachment(RootComponent);
	PlaceholderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlaceholderMesh->SetRelativeScale3D(FVector(0.58f, 0.58f, 1.25f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> EnemyMeshFinder(TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (EnemyMeshFinder.Succeeded())
	{
		PlaceholderMesh->SetStaticMesh(EnemyMeshFinder.Object);
	}

	AIControllerClass = AHellwakeEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

void AHellwakeEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(HellwakeTags::Event_Death)
			.AddUObject(this, &AHellwakeEnemyBase::OnDeathEvent);
	}

	if (EnemyDefinitionTable && !EnemyDefinitionRowName.IsNone())
	{
		if (const FHellwakeEnemyDefinition* Row = EnemyDefinitionTable->FindRow<FHellwakeEnemyDefinition>(EnemyDefinitionRowName, TEXT("HellwakeEnemyBase::BeginPlay")))
		{
			InitializeFromDefinition(*Row, EnemyDefinitionRowName);
		}
	}

	RingSlotRadians = FMath::FRandRange(0.f, PI * 2.f);
	AttackCooldownRemaining = FMath::FRandRange(0.6f, 1.6f);
}

void AHellwakeEnemyBase::InitializeFromDefinition(const FHellwakeEnemyDefinition& Definition, FName RowName)
{
	EnemyDefinitionRowName = RowName;
	RoleTag = Definition.RoleTag;
	MoveSpeedCm = Definition.MoveSpeedCm;
	AttackDamage = Definition.AttackDamage;
	AttackRangeCm = Definition.AttackRangeCm;
	AttackCooldownSeconds = Definition.AttackCooldownSeconds;
	EngagementRingCm = Definition.EngagementRingCm;
	LootDropChance = Definition.LootDropChance;
	XPReward = Definition.XPReward;
	bIsElite = Definition.bIsElite;
	DespawnTimeRemaining = bIsElite ? 2.6f : 0.9f;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = MoveSpeedCm;
	}

	if (PlaceholderMesh)
	{
		if (bIsElite || RoleTag == HellwakeTags::Enemy_Role_Gravewarden)
		{
			PlaceholderMesh->SetRelativeScale3D(FVector(1.35f, 1.35f, 2.3f));
		}
		else if (RoleTag == HellwakeTags::Enemy_Role_Wraith)
		{
			PlaceholderMesh->SetRelativeScale3D(FVector(0.45f, 0.45f, 0.95f));
		}
		else if (RoleTag == HellwakeTags::Enemy_Role_Acolyte)
		{
			PlaceholderMesh->SetRelativeScale3D(FVector(0.52f, 0.52f, 1.45f));
		}
		else
		{
			PlaceholderMesh->SetRelativeScale3D(FVector(0.62f, 0.62f, 1.2f));
		}
	}

	if (AbilitySystemComponent && AttributeSet)
	{
		AbilitySystemComponent->ApplyModToAttribute(UHellwakeAttributeSet::GetMaxHealthAttribute(), EGameplayModOp::Override, Definition.MaxHealth);
		AbilitySystemComponent->ApplyModToAttribute(UHellwakeAttributeSet::GetHealthAttribute(), EGameplayModOp::Override, Definition.MaxHealth);
	}
}

UAbilitySystemComponent* AHellwakeEnemyBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

AActor* AHellwakeEnemyBase::FindPrimaryTarget() const
{
	return UGameplayStatics::GetPlayerPawn(this, 0);
}

void AHellwakeEnemyBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsDead)
	{
		UpdateDeathTick(DeltaSeconds);
		return;
	}

	if (StaggerRemaining > 0.f)
	{
		StaggerRemaining -= DeltaSeconds;
		return;
	}

	AnimTime += DeltaSeconds;
	AActor* Target = FindPrimaryTarget();
	if (Target)
	{
		UpdateAI(DeltaSeconds, Target);
	}
}

void AHellwakeEnemyBase::UpdateAI(float DeltaSeconds, AActor* Target)
{
	const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	const FRotator FaceRotation = FRotator(0.f, ToTarget.Rotation().Yaw, 0.f);
	SetActorRotation(FaceRotation);

	const float EffectiveSlot = RoleTag == HellwakeTags::Enemy_Role_Wraith ? RingSlotRadians + AnimTime * 0.5f : RingSlotRadians;
	const FVector WantLocation = Target->GetActorLocation() + FVector(FMath::Sin(EffectiveSlot), FMath::Cos(EffectiveSlot), 0.f) * EngagementRingCm;
	const FVector ToWant = WantLocation - GetActorLocation();
	if (ToWant.Size2D() > 60.f)
	{
		AddMovementInput(ToWant.GetSafeNormal2D(), 1.f);
	}

	TryAttack(DeltaSeconds, Target);
}

void AHellwakeEnemyBase::TryAttack(float DeltaSeconds, AActor* Target)
{
	AttackCooldownRemaining -= DeltaSeconds;
	if (AttackCooldownRemaining > 0.f)
	{
		return;
	}

	const float Distance = FVector::Dist2D(Target->GetActorLocation(), GetActorLocation());
	if (Distance > AttackRangeCm + 60.f)
	{
		return;
	}

	AttackCooldownRemaining = AttackCooldownSeconds;

	if (RoleTag == HellwakeTags::Enemy_Role_Acolyte)
	{
		TWeakObjectPtr<AActor> WeakTarget = Target;
		TWeakObjectPtr<AHellwakeEnemyBase> WeakSelf = this;
		FTimerHandle BoltHandle;
		GetWorld()->GetTimerManager().SetTimer(BoltHandle, [WeakSelf, WeakTarget]()
		{
			if (WeakSelf.IsValid() && WeakTarget.IsValid() && !WeakSelf->IsDead())
			{
				const float Distance = FVector::Dist2D(WeakSelf->GetActorLocation(), WeakTarget->GetActorLocation());
				if (Distance < WeakSelf->AttackRangeCm + 120.f)
				{
					WeakSelf->DealDamageTo(WeakTarget.Get(), WeakSelf->AttackDamage);
				}
			}
		}, 0.85f, false);
	}
	else
	{
		TWeakObjectPtr<AActor> WeakTarget = Target;
		TWeakObjectPtr<AHellwakeEnemyBase> WeakSelf = this;
		FTimerHandle SwingHandle;
		GetWorld()->GetTimerManager().SetTimer(SwingHandle, [WeakSelf, WeakTarget]()
		{
			if (WeakSelf.IsValid() && WeakTarget.IsValid() && !WeakSelf->IsDead())
			{
				const float Distance = FVector::Dist2D(WeakSelf->GetActorLocation(), WeakTarget->GetActorLocation());
				if (Distance < WeakSelf->AttackRangeCm + 120.f)
				{
					WeakSelf->DealDamageTo(WeakTarget.Get(), WeakSelf->AttackDamage);
				}
			}
		}, 0.3f, false);
	}
}

void AHellwakeEnemyBase::DealDamageTo(AActor* Target, float Damage, bool bCrit)
{
	if (!Target || !DamageEffectClass || !AbilitySystemComponent)
	{
		return;
	}

	IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(Target);
	UAbilitySystemComponent* TargetASC = TargetASI ? TargetASI->GetAbilitySystemComponent() : nullptr;
	if (!TargetASC)
	{
		return;
	}

	FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
	Context.AddSourceObject(this);
	FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), Damage);
		if (bCrit)
		{
			Spec.Data->DynamicGrantedTags.AddTag(HellwakeTags::Event_Damage_Crit);
		}
		AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*Spec.Data, TargetASC);
	}
}

void AHellwakeEnemyBase::OnDeathEvent(const FGameplayEventData* Payload)
{
	HandleDeath();
}

void AHellwakeEnemyBase::HandleDeath()
{
	if (bIsDead)
	{
		return;
	}
	bIsDead = true;

	UE_LOG(LogHellwakeCombat, Log, TEXT("%s died (role %s)"), *GetName(), *RoleTag.ToString());

	if (!bIsElite && LootDropComponent)
	{
		LootDropComponent->RollAndMaybeDrop(LootDropChance, GetActorLocation());
	}

	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		if (IAbilitySystemInterface* PlayerASI = Cast<IAbilitySystemInterface>(PlayerPawn))
		{
			if (UAbilitySystemComponent* PlayerASC = PlayerASI->GetAbilitySystemComponent())
			{
				const UHellwakeAttributeSet* PlayerAttr = PlayerASC->GetSet<UHellwakeAttributeSet>();
				const float CurrentXP = PlayerAttr ? PlayerAttr->GetXP() : 0.f;
				PlayerASC->ApplyModToAttribute(UHellwakeAttributeSet::GetXPAttribute(), EGameplayModOp::Override,
					FMath::Min(100.f, CurrentXP + XPReward / 12.f));
			}
		}
	}

	SetActorEnableCollision(false);
}

void AHellwakeEnemyBase::UpdateDeathTick(float DeltaSeconds)
{
	AddActorWorldOffset(FVector(0.f, 0.f, -120.f * DeltaSeconds));
	AddActorLocalRotation(FRotator(0.f, 0.f, 110.f * DeltaSeconds));

	DespawnTimeRemaining -= DeltaSeconds;
	if (DespawnTimeRemaining <= 0.f)
	{
		Destroy();
	}
}
