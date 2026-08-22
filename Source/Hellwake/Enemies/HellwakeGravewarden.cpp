#include "Enemies/HellwakeGravewarden.h"
#include "Attributes/HellwakeAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Data/HellwakeEnemyDefinition.h"
#include "Loot/HellwakeLootPickup.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "HellwakeGameplayTags.h"
#include "Hellwake.h"

namespace
{
	FHellwakeEnemyDefinition MakeBossAddDefinition(const FName RowName)
	{
		FHellwakeEnemyDefinition Def;
		if (RowName == TEXT("wraith"))
		{
			Def.DisplayName = FText::FromString(TEXT("Cinder Wraith"));
			Def.RoleTag = HellwakeTags::Enemy_Role_Wraith;
			Def.MaxHealth = 110.f;
			Def.MoveSpeedCm = 720.f;
			Def.AttackDamage = 7.f;
			Def.AttackRangeCm = 240.f;
			Def.AttackCooldownSeconds = 1.1f;
			Def.EngagementRingCm = 260.f;
			Def.LootDropChance = 0.30f;
			Def.XPReward = 32.f;
		}
		else
		{
			Def.DisplayName = FText::FromString(TEXT("Ashbound Reaver"));
			Def.RoleTag = HellwakeTags::Enemy_Role_Reaver;
			Def.MaxHealth = 180.f;
			Def.MoveSpeedCm = 440.f;
			Def.AttackDamage = 11.f;
			Def.AttackRangeCm = 290.f;
			Def.AttackCooldownSeconds = 1.7f;
			Def.EngagementRingCm = 320.f;
			Def.LootDropChance = 0.35f;
			Def.XPReward = 40.f;
		}
		return Def;
	}
}

AHellwakeGravewarden::AHellwakeGravewarden()
{
	bIsElite = true;
	ReaverClass = AHellwakeEnemyBase::StaticClass();
	WraithClass = AHellwakeEnemyBase::StaticClass();
	LegendaryLootPickupClass = AHellwakeLootPickup::StaticClass();
}

void AHellwakeGravewarden::UpdateAI(float DeltaSeconds, AActor* Target)
{
	const UHellwakeAttributeSet* Attr = AbilitySystemComponent ? AbilitySystemComponent->GetSet<UHellwakeAttributeSet>() : nullptr;
	const float HpPct = (Attr && Attr->GetMaxHealth() > 0.f) ? Attr->GetHealth() / Attr->GetMaxHealth() : 1.f;

	const EHellwakeGravewardenPhase WantPhase = HpPct > Phase2ThresholdPct ? EHellwakeGravewardenPhase::Phase1
		: HpPct > Phase3ThresholdPct ? EHellwakeGravewardenPhase::Phase2 : EHellwakeGravewardenPhase::Phase3;
	if (WantPhase != CurrentPhase)
	{
		EnterPhase(WantPhase);
	}

	const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	SetActorRotation(FRotator(0.f, ToTarget.Rotation().Yaw, 0.f));

	if (CurrentPhase != EHellwakeGravewardenPhase::Phase1 &&
		ToTarget.Size2D() < SoulrendAuraRadiusCm &&
		FMath::FRand() < DeltaSeconds * SoulrendAuraProcChancePerSecond)
	{
		DealDamageTo(Target, SoulrendAuraDamagePerTick);
	}

	if (AttackCastTimeRemaining > 0.f)
	{
		AttackCastTimeRemaining -= DeltaSeconds;
		return;
	}

	const float Distance = ToTarget.Size2D();
	if (Distance > AttackRangeCm)
	{
		const float SpeedScale = CurrentPhase == EHellwakeGravewardenPhase::Phase3 ? 1.35f : 1.f;
		AddMovementInput(ToTarget.GetSafeNormal2D(), SpeedScale);
	}

	AttackCooldownRemaining -= DeltaSeconds;
	if (AttackCooldownRemaining <= 0.f)
	{
		ChooseAndBeginAttack(Target);
	}
}

void AHellwakeGravewarden::EnterPhase(EHellwakeGravewardenPhase NewPhase)
{
	CurrentPhase = NewPhase;

	UE_LOG(LogHellwakeAI, Log, TEXT("Gravewarden entering %s"),
		NewPhase == EHellwakeGravewardenPhase::Phase2 ? TEXT("Phase 2 (Soulrend Aura)") :
		NewPhase == EHellwakeGravewardenPhase::Phase3 ? TEXT("Phase 3 (Corrupted Flame)") : TEXT("Phase 1"));

	if (NewPhase == EHellwakeGravewardenPhase::Phase2)
	{
		SpawnAdds(ReaverClass, TEXT("reaver"), 3, 500.f);
	}
	else if (NewPhase == EHellwakeGravewardenPhase::Phase3)
	{
		SpawnAdds(WraithClass, TEXT("wraith"), 2, 600.f);
	}
}

void AHellwakeGravewarden::SpawnAdds(TSubclassOf<AHellwakeEnemyBase> EnemyClass, FName DefinitionRowName, int32 Count, float SideOffsetCm)
{
	if (!EnemyClass || !GetWorld())
	{
		return;
	}

	FHellwakeEnemyDefinition FallbackDef = MakeBossAddDefinition(DefinitionRowName);
	const FHellwakeEnemyDefinition* Definition = &FallbackDef;
	if (EnemyDefinitionTable)
	{
		if (const FHellwakeEnemyDefinition* DataRow = EnemyDefinitionTable->FindRow<FHellwakeEnemyDefinition>(DefinitionRowName, TEXT("Gravewarden::SpawnAdds")))
		{
			Definition = DataRow;
		}
	}

	for (int32 i = 0; i < Count; ++i)
	{
		const float OffsetX = (i - (Count - 1) / 2.f) * SideOffsetCm;
		const FVector SpawnLocation = GetActorLocation() + FVector(OffsetX, 500.f, 80.f);
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		if (AHellwakeEnemyBase* Add = GetWorld()->SpawnActor<AHellwakeEnemyBase>(EnemyClass, SpawnLocation, FRotator::ZeroRotator, Params))
		{
			Add->InitializeFromDefinition(*Definition, DefinitionRowName);
		}
	}
}

void AHellwakeGravewarden::ChooseAndBeginAttack(AActor* Target)
{
	const float Roll = FMath::FRand();
	const bool bPhase3 = CurrentPhase == EHellwakeGravewardenPhase::Phase3;
	const float Distance = FVector::Dist2D(Target->GetActorLocation(), GetActorLocation());

	TWeakObjectPtr<AHellwakeGravewarden> WeakSelf = this;
	TWeakObjectPtr<AActor> WeakTarget = Target;

	if (Distance < 800.f && Roll < SweepWeight)
	{
		AttackCastTimeRemaining = SweepTelegraphSeconds;
		AttackCooldownRemaining = bPhase3 ? 1.5f : 2.3f;
		const FVector Point = GetActorLocation() + GetActorForwardVector() * SweepRangeCm;
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, [WeakSelf, WeakTarget, Point]()
		{
			if (!WeakSelf.IsValid() || !WeakTarget.IsValid()) return;
			if (FVector::Dist2D(WeakTarget->GetActorLocation(), Point) < WeakSelf->SweepRadiusCm)
			{
				WeakSelf->DealDamageTo(WeakTarget.Get(), WeakSelf->SweepDamage);
			}
		}, SweepTelegraphSeconds, false);
	}
	else if (Roll < SweepWeight + SlamWeight)
	{
		AttackCastTimeRemaining = SlamTelegraphSeconds;
		AttackCooldownRemaining = bPhase3 ? 1.8f : 2.8f;
		const FVector Point = Target->GetActorLocation();
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, [WeakSelf, WeakTarget, Point]()
		{
			if (!WeakSelf.IsValid() || !WeakTarget.IsValid()) return;
			if (FVector::Dist2D(WeakTarget->GetActorLocation(), Point) < WeakSelf->SlamRadiusCm)
			{
				WeakSelf->DealDamageTo(WeakTarget.Get(), WeakSelf->SlamDamage);
			}
		}, SlamTelegraphSeconds, false);
	}
	else
	{
		AttackCastTimeRemaining = 1.2f;
		AttackCooldownRemaining = 3.4f;
		const int32 PillarCount = bPhase3 ? 5 : 3;
		for (int32 i = 0; i < PillarCount; ++i)
		{
			const float Angle = FMath::FRand() * PI * 2.f;
			const float Range = FMath::FRandRange(PillarMinRangeCm, PillarMaxRangeCm);
			const FVector Point = GetActorLocation() + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Range;
			const float Delay = 1.3f + i * 0.16f;
			FTimerHandle Handle;
			GetWorld()->GetTimerManager().SetTimer(Handle, [WeakSelf, WeakTarget, Point]()
			{
				if (!WeakSelf.IsValid() || !WeakTarget.IsValid()) return;
				if (FVector::Dist2D(WeakTarget->GetActorLocation(), Point) < WeakSelf->PillarRadiusCm)
				{
					WeakSelf->DealDamageTo(WeakTarget.Get(), WeakSelf->PillarDamage);
				}
			}, Delay, false);
		}
	}
}

void AHellwakeGravewarden::HandleDeath()
{
	if (bIsDead)
	{
		return;
	}

	Super::HandleDeath();

	if (LegendaryLootPickupClass && GetWorld())
	{
		const FVector DropLocation = GetActorLocation() + GetActorForwardVector() * 300.f + FVector(0.f, 0.f, 40.f);
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		if (AHellwakeLootPickup* Pickup = GetWorld()->SpawnActor<AHellwakeLootPickup>(LegendaryLootPickupClass, DropLocation, FRotator::ZeroRotator, Params))
		{
			Pickup->ConfigurePickup(TEXT("legendary"), nullptr);
		}
	}
}
