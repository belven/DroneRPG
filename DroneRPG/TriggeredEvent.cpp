#include "TriggeredEvent.h"
#include "FunctionLibrary.h"

ATriggeredEvent::ATriggeredEvent()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATriggeredEvent::TriggerEvent()
{
	// Do Nothing by default
}

void ATriggeredEvent::BeginPlay()
{
	Super::BeginPlay();	
}

void ATriggeredEvent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}