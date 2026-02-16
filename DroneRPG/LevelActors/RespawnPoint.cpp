#include "RespawnPoint.h"
#include "NavigationSystem.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "DroneRPG/DroneRPGCharacter.h"
#include "DroneRPG/Components/HealthComponent.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "DroneRPG/Controllers/DroneBaseAI.h"
#include "DroneRPG/GameModes/DroneRPGGameMode.h"
#include "Kismet/GameplayStatics.h"

ARespawnPoint::ARespawnPoint() : teamSize(6)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1;

	keyActorSize = 2000;

	respawnArea = CreateDefaultSubobject<USphereComponent>(TEXT("RespawnArea"));
	respawnArea->SetSphereRadius(GetSize());
	UFunctionLibrary::SetupOverlap(respawnArea);
	RootComponent = respawnArea;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> auraParticleSystem(TEXT("/Game/TopDownCPP/ParticleEffects/AuraSystem_2"));

	if (auraParticleSystem.Succeeded()) {
		auraSystem = auraParticleSystem.Object;
	}
}

void ARespawnPoint::SpawnTeam()
{
	for (int i = 0; i < teamSize - 1; ++i)
	{
		FNavLocation loc;
		mRandomPointInNavigableRadius(GetActorLocation(), GetSize(), loc);

		FActorSpawnParameters params;
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		ADroneRPGCharacter* npc = GetWorld()->SpawnActor<ADroneRPGCharacter>(ADroneRPGCharacter::StaticClass(), loc.Location, GetActorRotation(), params);

		if (IsValid(npc))
		{
			npc->SetTeam(GetTeam());
			ADroneBaseAI* aiCon = GetWorld()->SpawnActor<ADroneBaseAI>(ADroneBaseAI::StaticClass());
			if (IsValid(aiCon))
			{
				aiCon->Possess(npc);
			}
		}
	}
}

void ARespawnPoint::SetupParticles()
{
	// Create our particle system
	captureParticle = UNiagaraFunctionLibrary::SpawnSystemAttached(auraSystem, RootComponent, TEXT("captureParticle"), FVector(1), FRotator(1), EAttachLocation::SnapToTarget, false);

	ADroneRPGGameMode* gameMode = Cast<ADroneRPGGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	FColor teamColour = gameMode->GetTeamColour(GetTeam());

	// Set up the systems defaults
	captureParticle->SetVectorParameter(TEXT("Box Extent"), FVector(GetSize(), GetSize(), 400));
	captureParticle->SetFloatParameter(TEXT("Size"), 200);
	captureParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(teamColour));
}

void ARespawnPoint::BeginPlay()
{
#if WITH_EDITOR
	SetFolderPath(TEXT("Respawn Points"));
#endif

	Super::BeginPlay();
	SetupParticles();
	respawnArea->OnComponentBeginOverlap.AddDynamic(this, &ARespawnPoint::BeginOverlap);
}

void ARespawnPoint::BeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	FTargetData data = UCombatClasses::CreateTargetData(OtherActor);
	if (data.isSet)
	{
		if (data.GetTeam() == GetTeam() && !data.healthComponent->IsHealthy())
		{
			data.healthComponent->FullHeal();
		}
	}
}