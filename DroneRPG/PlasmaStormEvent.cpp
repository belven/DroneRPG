#include "PlasmaStormEvent.h"
#include "DroneRPGCharacter.h"
#include "FunctionLibrary.h"
#include "NavigationSystem.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "../Plugins/FX/Niagara/Source/Niagara/Classes/NiagaraSystem.h"
#include "Enums.h"

#define mSpawnSystemAttached(system, name) UNiagaraFunctionLibrary::SpawnSystemAttached(system, meshComponent, name, FVector(0,0, 1000), FRotator(1), EAttachLocation::KeepRelativeOffset, false)

APlasmaStormEvent::APlasmaStormEvent()
{
	radius = 2000.0f;
	powerDrainLimit = radius * 3;
	damage = 20.0f;
	travelDistance = 20000;
	damageRate = 3.0f;
	acceleration = 0.05f;
	moveRate = 10.0f;
	targetLocation.Location = FVector::ZeroVector;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> auraParticleSystem(TEXT("/Game/TopDownCPP/ParticleEffects/Plasma_Storm"));

	if (auraParticleSystem.Succeeded()) {
		stormSystem = auraParticleSystem.Object;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> sphereMesh(TEXT("StaticMesh'/Game/TopDownCPP/Models/Shape_Sphere'"));

	meshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StormMesh"));
	if (sphereMesh.Succeeded())
	{
		meshComponent->SetStaticMesh(sphereMesh.Object);
		meshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		meshComponent->SetupAttachment(RootComponent);
		meshComponent->SetHiddenInGame(true);
	}
}

void APlasmaStormEvent::DroneKilled(ADroneRPGCharacter* drone)
{
	kills++;
}

FString APlasmaStormEvent::GetDamagerName()
{
	return GetEventName();
}

EDamagerType APlasmaStormEvent::GetDamagerType()
{
	return EDamagerType::PlasmaStorm;
}

FString APlasmaStormEvent::GetEventName()
{
	return "Plasma Storm";
}

void APlasmaStormEvent::TriggerEvent()
{
	// Get drones within our radius, deal damage to them
	for (ADroneRPGCharacter* drone : mGetDronesInRadius(GetRadius(), GetActorLocation())) {
		drone->DamageDrone(damage, this);
		damageDealt += damage;
	}

	// Set the timer that runs this method on loop
	mSetTimer(TimerHandle_EventTrigger, &APlasmaStormEvent::TriggerEvent, damageRate);
}

void APlasmaStormEvent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update our location every tick, moving a rate towards our target location
	SetActorLocation(FMath::Lerp(GetActorLocation(), targetLocation.Location, acceleration));

	// If we our a Power Drainer then, update our radius, as it may have changed
	if (isPowerDrainer) {
		stormParticle->SetFloatParameter(TEXT("Radius"), GetRadius());
	}
}

void APlasmaStormEvent::BeginPlay()
{
	Super::BeginPlay();
	damageDealt = 0.0f;
	stormParticle = mSpawnSystemAttached(stormSystem, TEXT("Storm Particles"));
	stormParticle->SetFloatParameter(TEXT("Radius"), GetRadius());
	stormParticle->SetColorParameter(TEXT("Colour"), FLinearColor(FColor::Purple));
	stormParticle->SetFloatParameter(TEXT("Size"), 175);

	TriggerEvent();
	Move();
}

float APlasmaStormEvent::GetRadius()
{
	// Radius can be influenced by damage done if we are a Power Drainer
	return isPowerDrainer ? MIN(radius + damageDealt, powerDrainLimit) : radius;
}

void APlasmaStormEvent::Move()
{
	// We're not a Player hunter, find a random location to move to, within a radius
	if (!isPlayerHunter) {
		int32 count = 0;
		mRandomPointInNavigableRadius(GetActorLocation(), travelDistance, targetLocation);

		while (mDist(GetActorLocation(), targetLocation.Location) <= travelDistance * 0.7 && count <= 30)
		{
			count++;
			mRandomPointInNavigableRadius(GetActorLocation(), travelDistance, targetLocation);
		}
	}
	// Otherwise, find a random player and move to thier location
	else if(mGetDrones.Num() > 0) {
		targetLocation.Location = mGetRandomObject(mGetDrones)->GetActorLocation();
	}

	mSetTimer(TimerHandle_Move, &APlasmaStormEvent::Move, moveRate);
}
