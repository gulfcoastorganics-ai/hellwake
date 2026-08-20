#include "Enemies/HellwakeEnemyAIController.h"
#include "Hellwake.h"

AHellwakeEnemyAIController::AHellwakeEnemyAIController()
{
}

void AHellwakeEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	UE_LOG(LogHellwakeAI, Verbose, TEXT("%s possessed %s"), *GetName(), InPawn ? *InPawn->GetName() : TEXT("null"));
}
