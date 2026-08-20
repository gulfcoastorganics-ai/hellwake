// AI controller for Reaver/Wraith/Acolyte. Currently a thin possessor —
// the actual per-frame behavior lives in AHellwakeEnemyBase::UpdateAI (see
// that class's header comment for why: it's a verified, testable port of
// the prototype's enemyAI(), and the intended upgrade path once the Editor
// is available is to replace it with a State Tree run from this controller
// (add a UStateTreeAIComponent here, assign DT_Enemy_Default, and have its
// tasks call the same AHellwakeEnemyBase Blueprint-callable functions:
// TryAttack, DealDamageTo, the engagement-ring query) rather than rewriting
// combat resolution from scratch. See project-bible/enemies.md.
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "HellwakeEnemyAIController.generated.h"

UCLASS()
class HELLWAKE_API AHellwakeEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AHellwakeEnemyAIController();

	virtual void OnPossess(APawn* InPawn) override;
};
