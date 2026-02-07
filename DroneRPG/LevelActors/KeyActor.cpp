#include "KeyActor.h"

AKeyActor::AKeyActor(): keyActorSize(0)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AKeyActor::BeginPlay()
{
	Super::BeginPlay();	
}

void AKeyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}