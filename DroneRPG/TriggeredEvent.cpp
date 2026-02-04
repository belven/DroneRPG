#include "TriggeredEvent.h"

ATriggeredEvent::ATriggeredEvent()
{
	PrimaryActorTick.bCanEverTick = true;
}

FString ATriggeredEvent::GetEventName()
{
	return "Unset";
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