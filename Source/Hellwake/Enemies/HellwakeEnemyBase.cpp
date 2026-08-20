#include "Enemies/HellwakeEnemyBase.h"
#include "AbilitySystemComponent.h"
#include "Attributes/HellwakeAttributeSet.h"
#include "Combat/HellwakeVitalityComponent.h"
#include "Loot/HellwakeLootDropComponent.h"
#include "Data/HellwakeEnemyDefinition.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
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

	// Enemies always face their target directly, unlike the player (whose
	// facing is movement-driven). Prototype: `e.obj.rotation.y =
	// atan2(d.x, d.z)` every frame, unconditionally.
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
	const float Distance = ToTarget.Size2D();

	// Face the target directly, every frame.
	const FRotator FaceRotation = FRotator(0.f, ToTarget.Rotation().Yaw, 0.f);
	SetActorRotation(FaceRotation);

	// Ring-hold positioning: try to sit at EngagementRingCm from the
	// target, at this enemy's slot angle around it. Prototype:
	// `want = hero.position + (sin(slot), cos(slot)) * def.ring`.
	// Cinder Wraiths additionally orbit: `slot + anim*0.5`.
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

	// Ranged (Pyre Acolyte) telegraphs at the target's position at cast
	// time, matching the prototype's `telegraph(px, pz, 2.6, 0.85, ...)`.
	// The base implementation here resolves it after a short delay via
	// timer rather than a full telegraph-VFX task; see
	// project-bible/enemies.md for the in-editor Niagara telegraph spec.
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
				if (Distance < WeakSelf->AttackRangeCm + 60.f + 60.f)
				{
					WeakSelf->DealDamageTo(WeakTarget.Get(), WeakSelf->AttackDamage);
				}
			}
		}, 0.85f, false);
	}
	else
	{
		// Melee: prototype delays contact by 0.3s to match the swing anim.
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

	// Prototype: `P.xp = Math.min(100, P.xp + e.def.xp/12)` on every kill,
	// unconditional of who/what landed the killing blow (single-player slice).
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

	// TODO(VFX): burst + shockwave at death location, scaled for elites.

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
