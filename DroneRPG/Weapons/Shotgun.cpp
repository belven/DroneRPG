#pragma once
#include "Shotgun.h"

#include <DroneRPG/Components/CombatantComponent.h>

#include "DroneProjectile.h"
#include <Kismet/GameplayStatics.h>
#include "DroneRPG/DroneRPGCharacter.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"


UShotgun::UShotgun(): pellets(0)
{
	weaponType = EWeaponType::Shotgun;
	spread = 0.15f;
	lifespan = 0.7f;
	speed = ADroneProjectile::Default_Initial_Speed;
}

UShotgun* UShotgun::CreateShotgun(float inFireRate, float inDamage, UCombatantComponent* inOwner, int32 inPellets)
{
	UShotgun* weapon = NewObject<UShotgun>(StaticClass());
	weapon->fireRate = inFireRate;
	weapon->damage = inDamage;
	weapon->owner = inOwner;
	weapon->pellets = inPellets;
	return weapon;
}

FRotator UShotgun::RandomDirection(FRotator fireRotation) {
	float lower = fireRotation.Yaw * (1 - spread);
	float upper = fireRotation.Yaw * (1 + spread);
	fireRotation.Yaw = FMath::RandRange(lower, upper);
	return fireRotation;
}

void UShotgun::FireShot(FVector FireDirection)
{
	if (canFire)
	{
		if (FireDirection.SizeSquared() > 0.0f)
		{
			for (int i = 0; i < pellets; i++) {
				const FRotator FireRotation = RandomDirection(FireDirection.Rotation());
				const FVector gunLocation = owner->GetOwner()->GetActorLocation() + FireRotation.RotateVector(GunOffset);

				SpawnProjectile(gunLocation, FireRotation);
				mSetTimerWorld(owner->GetWorld(), TimerHandle_ShotTimerExpired, &UShotgun::ShotTimerExpired, fireRate);
			}

			if (FireSound != nullptr)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FireSound, owner->GetOwner()->GetActorLocation());
			}

			canFire = false;
		}
	}
}

