#include "PlasmaStormEvent.h"
#include "../Plugins/FX/Niagara/Source/Niagara/Classes/NiagaraSystem.h"
#include "Components/SphereComponent.h"
#include "DroneRPG/GameModes/DroneRPGGameMode.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include <DroneRPG/Components/CombatantComponent.h>
#include "DroneRPG/Components/HealthComponent.h"
#include "DroneRPG/Utilities/CombatClasses.h"

#define mSpawnSystemAttached(system, name) UNiagaraFunctionLibrary::SpawnSystemAttached(system, meshComponent, name, FVector(0,0, 1000), FRotator(1), EAttachLocation::KeepRelativeOffset, false)

APlasmaStormEvent::APlasmaStormEvent()
{
#if WITH_EDITOR
	if (IsRunningGame())
		SetFolderPath(TEXT("Environmental Effects"));
#endif
	radius = 750.f;
	powerDrainLimit = radius * 3;
	damage = 40.0f;
	travelDistance = 10000;
	damageRate = 2.0f;
	acceleration = 0.001f;
	moveRate = 10.0f;
	targetLocation.Location = FVector::ZeroVector;
	isPlayerHunter = true;
	isPowerDrainer = true;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> auraParticleSystem(TEXT("/Game/TopDownCPP/ParticleEffects/Plasma_Storm"));

	if (auraParticleSystem.Succeeded()) {
		stormSystem = auraParticleSystem.Object;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> sphereMesh(TEXT("StaticMesh'/Game/TopDownCPP/Models/Shape_Sphere'"));

	//TODO Change to using a Sphere comp and use overlap to track actors
	meshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StormMesh"));
	if (sphereMesh.Succeeded())
	{
		meshComponent->SetStaticMesh(sphereMesh.Object);
		meshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		meshComponent->SetupAttachment(RootComponent);
		meshComponent->SetHiddenInGame(true);
	}

	sphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("PlasmaStormArea"));
	sphereComponent->SetSphereRadius(radius);
	sphereComponent->SetupAttachment(GetRootComponent());
	sphereComponent->SetCollisionResponseToAllChannels(ECR_Overlap);

	combatantComponent = CreateDefaultSubobject<UCombatantComponent>(TEXT("CombatComp"));
	combatantComponent->SetupCombatantComponent("Plasma Storm", EDamagerType::PlasmaStorm);
	combatantComponent->SetTeam(-1);
	combatantComponent->OnUnitKilled.AddUniqueDynamic(this, &APlasmaStormEvent::UnitKilled);
}

void APlasmaStormEvent::UnitKilled(UCombatantComponent* unitKilled)
{
	kills++;
}

void APlasmaStormEvent::TriggerEvent()
{
	TArray<AActor*> overlaps;
	GetOverlappingActors(overlaps);

	// Get drones within our radius, deal damage to them
	for (auto actor : overlaps)
	{
		UHealthComponent* healthComponent = mGetHealthComponent(actor);
		if (IsValid(healthComponent))
		{
			healthComponent->ReceiveDamage(damage, combatantComponent);
			damageDealt += damage;
		}
	}

	// Set the timer that runs this method on loop
	mSetTimer(TimerHandle_EventTrigger, &APlasmaStormEvent::TriggerEvent, damageRate);
}

void APlasmaStormEvent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update our location every tick, moving a rate towards our target location
	SetActorLocation(FMath::Lerp(GetActorLocation(), targetLocation.Location, acceleration));

	// If we are a Power Drainer then, update our radius, as it may have changed
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
	if (!isPlayerHunter)
	{
		mRandomPointInNavigableRadius(GetActorLocation(), travelDistance, targetLocation);
	}
	// Otherwise, find a random player and move to their location
	else if (GetGameMode()->GetCombatants().Num() > 0)
	{
		UCombatantComponent* combatant = NULL;

		while (!IsValid(combatant))
		{
			combatant = mGetRandomObject(GetGameMode()->GetCombatants());
		}

		targetLocation.Location = combatant->GetOwner()->GetActorLocation();
	}

	mSetTimer(TimerHandle_Move, &APlasmaStormEvent::Move, moveRate);
}

ADroneRPGGameMode* APlasmaStormEvent::GetGameMode()
{
	if (!IsValid(gameMode))
	{
		gameMode = Cast<ADroneRPGGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	}
	return gameMode;
}