#include "Rocket.h"
#include "FunctionLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DroneRPGCharacter.h"
#include <Kismet/KismetMathLibrary.h>
#include "../Plugins/FX/Niagara/Source/Niagara/Classes/NiagaraSystem.h"

const float ARocket::Default_Initial_Speed = 1000.0f;
const float ARocket::Default_Initial_Lifespan = 3.5f;

ARocket::ARocket() : Super()
{
	const float speed = 3000.0f;
	ProjectileMovement->InitialSpeed = Default_Initial_Speed;
	ProjectileMovement->MaxSpeed = speed;
	ProjectileMovement->HomingAccelerationMagnitude = 15000;
	InitialLifeSpan = Default_Initial_Lifespan;
	canCheckForEnemies = true;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> rocketTrailSystem(TEXT("NiagaraSystem'/Game/TopDownCPP/ParticleEffects/Rocket_Trail.Rocket_Trail'"));

	if (rocketTrailSystem.Succeeded()) {
		trailSystem = rocketTrailSystem.Object;
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

void ARocket::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (canCheckForEnemies) {
		mSetTimer(TimerHandle_CanCheckForEnemies, &ARocket::CanCheckForEnemies, 0.3f);
		canCheckForEnemies = false;
		FindTarget();
	}

	//if (target != NULL && ProjectileMovement->HomingTargetComponent == NULL) {
		//FRotator lookAt = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), target->GetActorLocation());
		//lookAt.Pitch = GetActorRotation().Pitch;
		//lookAt.Roll = GetActorRotation().Roll;
		//SetActorRotation(lookAt);
		//FVector force = GetActorForwardVector();
		//force.Normalize();
		//force * 2;
		//ProjectileMovement->AddForce(force);
	//}
}

void ARocket::FindTarget()
{
	if ((target == NULL || !target->IsAlive()) && GetShooter() != NULL) {
		SetTarget(mGetClosestEnemyInRadius(4000, GetActorLocation(), GetShooter()->GetTeam()));
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
						drone->RecieveHit(this);
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

void ARocket::CanCheckForEnemies()
{
	canCheckForEnemies = true;
}
