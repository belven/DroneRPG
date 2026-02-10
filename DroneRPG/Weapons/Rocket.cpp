#include "Rocket.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "../Plugins/FX/Niagara/Source/Niagara/Classes/NiagaraSystem.h"
#include "Components/SphereComponent.h"
#include "DroneRPG/DroneRPGCharacter.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"

const float ARocket::Default_Initial_Speed = 2500.0f;
const float ARocket::Default_Initial_Lifespan = 4.5f;

ARocket::ARocket()
{
	team = -1;
	ProjectileMovement->InitialSpeed = Default_Initial_Speed;
	ProjectileMovement->MaxSpeed = 3500.0f;
	ProjectileMovement->HomingAccelerationMagnitude = 15000;
	InitialLifeSpan = Default_Initial_Lifespan;

	static ConstructorHelpers::FObjectFinder<USoundBase> FireSoundAudio(TEXT("SoundCue'/Game/TopDownCPP/Sounds/Rocket_Booster_Cue.Rocket_Booster_Cue'"));
	FireSound = FireSoundAudio.Object;

	static ConstructorHelpers::FObjectFinder<USoundBase>HitSoundAudio(TEXT("SoundCue'/Game/TopDownCPP/Sounds/Rocket_Explosion_Cue.Rocket_Explosion_Cue'"));
	HitSound = HitSoundAudio.Object;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> rocketTrailSystem(TEXT("NiagaraSystem'/Game/TopDownCPP/ParticleEffects/Rocket_Trail.Rocket_Trail'"));

	if (rocketTrailSystem.Succeeded()) {
		trailSystem = rocketTrailSystem.Object;
	}

	sphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RocketOverlap"));
	sphereComponent->SetSphereRadius(2000);
	sphereComponent->SetupAttachment(GetRootComponent());
	sphereComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	sphereComponent->OnComponentBeginOverlap.AddUniqueDynamic(this, &ARocket::BeginOverlap);
}

bool ARocket::CheckIfValidTarget(ADroneRPGCharacter* droneFound)
{
	return IsValid(droneFound) && droneFound->GetHealthComponent()->IsAlive() && droneFound->GetTeam() != team;
}

void ARocket::SetTeam(int32 inTeam)
{
	team = inTeam;

	TArray<AActor*> overlaps;
	sphereComponent->GetOverlappingActors(overlaps);

	for (auto overlap : overlaps)
	{

		ADroneRPGCharacter* droneFound = Cast<ADroneRPGCharacter>(overlap);
		if (CheckIfValidTarget(droneFound))
		{
			target = droneFound;
		}
	}
}

void ARocket::SetTarget(ADroneRPGCharacter* val)
{
	Super::SetTarget(val);

	if (target != NULL) {
		ProjectileMovement->bIsHomingProjectile = true;
		ProjectileMovement->HomingTargetComponent = target->GetRootComponent();
	}
}

void ARocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if (OtherActor != NULL && OtherActor != this && OtherActor != shooter && !mIsA(OtherActor, ADroneProjectile))
	{
		ADroneRPGCharacter* droneHit = Cast<ADroneRPGCharacter>(OtherActor);

		// Did we hit a drone?
		if (droneHit != NULL) {
			TArray<ADroneRPGCharacter*> drones = mGetActorsInWorld<ADroneRPGCharacter>(GetWorld());

			for (ADroneRPGCharacter* drone : drones)
			{
				// Check if the distance is within the given range
				if (mDist(droneHit->GetActorLocation(), drone->GetActorLocation()) <= 1000) {

					// Check if the drone found is an enemy
					if (GetShooter()->GetTeam() != drone->GetTeam()) {
						// Deal damage to enemy Drone
						drone->GetHealthComponent()->ReceiveHit(this);
						Destroy();
					}
				}
			}
		}
		else {
			Destroy();
		}
	}
}

void ARocket::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (team != -1) 
	{
		ADroneRPGCharacter* droneFound = Cast<ADroneRPGCharacter>(OtherActor);

		if ((target == NULL || !target->GetHealthComponent()->IsAlive()) && CheckIfValidTarget(droneFound))
		{
			SetTarget(droneFound);
		}
	}
}