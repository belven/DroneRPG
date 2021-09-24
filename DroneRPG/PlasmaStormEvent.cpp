#include "PlasmaStormEvent.h"
#include "DroneRPGCharacter.h"
#include "FunctionLibrary.h"
#include "NavigationSystem.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "../Plugins/FX/Niagara/Source/Niagara/Classes/NiagaraSystem.h"

#define mSpawnSystemAttached(system, name) UNiagaraFunctionLibrary::SpawnSystemAttached(system, meshComponent, name, FVector(1), FRotator(1), EAttachLocation::SnapToTarget, false)

APlasmaStormEvent::APlasmaStormEvent()
{
	radius = 2000.0f;
	damage = 20.0f;
	travelDistance = 20000;
	damageRate = 3.0f;
	acceleration = 0.05f;
	moveRate = 10.0f;
	targetLocation.Location = FVector::ZeroVector;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> auraParticleSystem(TEXT("/Game/TopDownCPP/ParticleEffects/AuraSystem"));

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
	}
}

void APlasmaStormEvent::TriggerEvent()
{
	for (ADroneRPGCharacter* drone : mGetDronesInRadius(radius, GetActorLocation())) {
		drone->DamageDrone(damage);
		damageDealt += damage;
	}

	mSetTimer(TimerHandle_EventTrigger, &APlasmaStormEvent::TriggerEvent, damageRate);
}

void APlasmaStormEvent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetActorLocation(FMath::Lerp(GetActorLocation(), targetLocation.Location, acceleration));
}

void APlasmaStormEvent::BeginPlay()
{
	Super::BeginPlay();
	damageDealt = 0.0f;
	stormParticle = mSpawnSystemAttached(stormSystem, TEXT("Storm Particles"));
	stormParticle->SetFloatParameter(TEXT("Radius"), radius);
	stormParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Green));
	stormParticle->SetFloatParameter(TEXT("Size"), 40);

	TriggerEvent();
	Move();
}

void APlasmaStormEvent::Move()
{
	int32 count = 0;
	mRandomReachablePointInRadius(GetActorLocation(), travelDistance, targetLocation);

	while (mDist(GetActorLocation(), targetLocation.Location) <= travelDistance * 0.7 && count <= 30)
	{
		count++;
		mRandomReachablePointInRadius(GetActorLocation(), travelDistance, targetLocation);
	}

	mSetTimer(TimerHandle_Move, &APlasmaStormEvent::Move, moveRate);
}
