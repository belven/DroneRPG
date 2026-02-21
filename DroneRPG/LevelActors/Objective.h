#pragma once
#include "CoreMinimal.h"
#include "KeyActor.h"
#include "Objective.generated.h"

class UObjectiveComponent;

UCLASS()
class DRONERPG_API AObjective : public AKeyActor
{
	GENERATED_BODY()

public:
	AObjective();

	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UObjectiveComponent* objectiveComponent;
};
