#pragma once
#include "Shotgun.h"
#include "DroneProjectile.h"
#include <Kismet/GameplayStatics.h>
#include "DroneRPG/DroneRPGCharacter.h"
#include "DroneRPG/FunctionLibrary.h"

const float UShotgun::Default_Initial_Lifespan = 0.7f;

UShotgun::UShotgun(): pellets(0)
{
	weaponType = EWeaponType::Shotgun;
	spread = 0.15f;
}

float UShotgun::GetRange()
{
	return ADroneProjectile::Default_Initial_Speed * Default_Initial_Lifespan;
}

UShotgun* UShotgun::CreateShotgun(float inFireRate, float inDamage, ADroneRPGCharacter* inOwner, int32 inPellets)
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

void UShotgun::FireShot(FVector FireDirection, AActor* target)
{
	if (canFire)
	{
		if (FireDirection.SizeSquared() > 0.0f)
		{
			for (int i = 0; i < pellets; i++) {
				const FRotator FireRotation = RandomDirection(FireDirection.Rotation());
				const FVector gunLocation = owner->GetActorLocation() + FireRotation.RotateVector(GunOffset);

				SpawnProjectile(gunLocation, FireRotation, target);
				mSetTimerWorld(owner->GetWorld(), TimerHandle_ShotTimerExpired, &UShotgun::ShotTimerExpired, fireRate);
			}

			if (FireSound != nullptr)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FireSound, owner->GetActorLocation());
			}

			canFire = false;
		}
	}
}

ADroneProjectile* UShotgun::SpawnProjectile(FVector gunLocation, FRotator FireRotation, AActor* target)
{
	ADroneProjectile* proj = Super::SpawnProjectile(gunLocation, FireRotation, target);
	proj->SetLifeSpan(Default_Initial_Lifespan);
	return proj;
}
