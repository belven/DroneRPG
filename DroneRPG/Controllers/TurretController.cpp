#include "TurretController.h"
#include "DroneRPG/DroneRPG.h"
#include "DroneRPG/Actors/Turret.h"
#include "DroneRPG/Weapons/Weapon.h"
#include "Kismet/KismetMathLibrary.h"

void ATurretController::SetFiringState(bool firingState)
{
	Super::SetFiringState(firingState);

	if (GetTurret()->GetWeapon()->IsActive() != firingState)
	{
		UE_LOG(LogDroneAI, Log, TEXT("%s shooting at %s"), *GetTurret()->GetCharacterName(), *GetTarget().GetCombatantName());
		GetTurret()->GetWeapon()->SetActive(firingState);
	}
}

void ATurretController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// We don't do anything if we're dead!
	if (GetTurret()->GetHealthComponent()->IsAlive())
	{
		RotateToFace(DeltaSeconds);

		if (IsTargetValid())
		{
			SetFiringState(true);
		}
		else
		{
			SetFiringState(false);
		}
	}
	else
	{
		SetFiringState(false);
	}
}

FVector ATurretController::GetPredictedLocation(AActor* actor)
{
	FVector v2 = actor->GetActorLocation();
	FVector v1 = GetPawn()->GetActorLocation();
	UWeapon* weapon = GetTurret()->GetWeapon();
	float time = FVector::Dist(v1, v2) / weapon->GetProjectileSpeed();
	return v2 + (actor->GetVelocity() * time);
}

void ATurretController::RotateToFace(float DeltaSeconds)
{
	FVector targetLocation;

	// If we have a target, turn to face it
	if (GetTarget().isSet && GetTarget().IsAlive())
	{
		targetLocation = GetPredictedLocation(GetTarget());
	}

	FRotator lookAt = UKismetMathLibrary::FindLookAtRotation(GetPawn()->GetActorLocation(), targetLocation);
	 
	lookAt.Pitch = 0;
	lookAt.Roll = 0;

	GetPawn()->SetActorRotation(FMath::RInterpTo(GetPawn()->GetActorRotation(), lookAt, DeltaSeconds, 8.0f));
}

ATurret* ATurretController::GetTurret()
{
	if (!IsValid(turret))
	{
		turret = Cast<ATurret>(GetCharacter());
	}
	return turret;
}
