#include "Shotgun.h"
#include "DroneProjectile.h"
#include "DroneRPGCharacter.h"
#include <Kismet/GameplayStatics.h>
#include "FunctionLibrary.h"

UShotgun::UShotgun()
{
	weaponType = EWeaponType::Shotgun;
}

UShotgun* UShotgun::CreateShotgun(float inFireRate, float inDamage, ADroneRPGCharacter* inOwner)
{
	UShotgun* weapon = NewObject<UShotgun>(UShotgun::StaticClass());

	weapon->fireRate = inFireRate;
	weapon->damage = inDamage;
	weapon->owner = inOwner;

	return weapon;
}

FRotator UShotgun::RandomDirection(FRotator fireRotation) {
	fireRotation.Yaw = FMath::RandRange(fireRotation.Yaw * 0.8f, fireRotation.Yaw * 1.2f);
	return fireRotation;
}

void UShotgun::FireShot(FVector FireDirection)
{
	if (canFire)
	{
		if (FireDirection.SizeSquared() > 0.0f)
		{
			for (int i = 0; i < 5; i++) {
				const FRotator FireRotation = RandomDirection(FireDirection.Rotation());
				const FVector gunLocation = owner->GetActorLocation() + FireRotation.RotateVector(GunOffset);

				ADroneProjectile* projectile = mSpawnProjectile;
				projectile->SetShooter(owner);
				projectile->SetDamage(FMath::RandRange(damage * 0.95f, damage * 1.05f));
				projectile->SetLifeSpan(0.5f);
				mSetTimerWolrd(owner->GetWorld(), TimerHandle_ShotTimerExpired, &UShotgun::ShotTimerExpired, fireRate);
			}

			if (FireSound != nullptr)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FireSound, owner->GetActorLocation());
			}

			canFire = false;
		}
	}
}