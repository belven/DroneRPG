#include "Objective.h"
#include "DroneRPG/Components/ObjectiveComponent.h"

AObjective::AObjective()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1;

	keyActorSize = 2000;

	Tags.Add("Objective");

	objectiveComponent = CreateDefaultSubobject<UObjectiveComponent>("ObjectiveComp");
	objectiveComponent->SetSize(keyActorSize);
}

void AObjective::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
	SetFolderPath(TEXT("Objectives"));
#endif
}