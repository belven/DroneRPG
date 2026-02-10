#include "RespawnPoint.h"

#include "NavigationSystem.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "DroneRPG/DroneRPGCharacter.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "DroneRPG/Controllers/DroneBaseAI.h"

ARespawnPoint::ARespawnPoint() : teamSize(6)
{
#if WITH_EDITOR
		SetFolderPath(TEXT("Respawn Points"));
#endif

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1;

	keyActorSize = 1000;

	respawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("RespawnArea"));
	respawnArea->SetBoxExtent(FVector(GetSize(), GetSize(), 400));
	respawnArea->SetupAttachment(GetRootComponent());

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

	FColor teamColour = UFunctionLibrary::GetTeamColour(GetTeam());

	// Set up the systems defaults
	captureParticle->SetVectorParameter(TEXT("Box Extent"), FVector(GetSize(), GetSize(), 400));
	captureParticle->SetFloatParameter(TEXT("Size"), 200);
	captureParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(teamColour));
}

void ARespawnPoint::BeginPlay()
{
	Super::BeginPlay();
	respawnArea->OnComponentBeginOverlap.AddDynamic(this, &ARespawnPoint::BeginOverlap);
	respawnArea->OnComponentEndOverlap.AddDynamic(this, &ARespawnPoint::EndOverlap);
}

void ARespawnPoint::BeginOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (mIsA(OtherActor, ADroneRPGCharacter)) {
		ADroneRPGCharacter* drone = Cast<ADroneRPGCharacter>(OtherActor);
		if (drone->GetTeam() == GetTeam() && !drone->GetHealthComponent()->IsHealthy()) {
			drone->GetHealthComponent()->FullHeal();
		}
	}
}

void ARespawnPoint::EndOverlap(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{

}

void ARespawnPoint::RespawnCharacter(ADroneRPGCharacter* character) {

}

void ARespawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}