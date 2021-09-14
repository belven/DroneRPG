#include "Weapon.h"
#include "DroneProjectile.h"
#include "DroneRPGCharacter.h"
#include <Kismet/GameplayStatics.h>
#include "FunctionLibrary.h"


UWeapon::UWeapon()
{
	fireRate = 0.3f;
	damage = 15.0f;
	canFire = true;
	weaponType = EWeaponType::Laser;
	GunOffset = FVector(100.f, 0.f, 0.f);

	static ConstructorHelpers::FClassFinder<ADroneProjectile> ProjectileClassFound(TEXT("/Game/TopDownCPP/Blueprints/Projectiles/Base"));

	if (ProjectileClassFound.Succeeded()) {
		projectileClass = ProjectileClassFound.Class;
	}
}

UWeapon* UWeapon::CreateWeapon(float inFireRate, float inDamage, ADroneRPGCharacter* inOwner)
{
	UWeapon* weapon = NewObject<UWeapon>(UWeapon::StaticClass());

	weapon->fireRate = inFireRate;
	weapon->damage = inDamage;
	weapon->owner = inOwner;

	return weapon;
}

void UWeapon::ShotTimerExpired()
{
	canFire = true;
}

void UWeapon::FireShot(FVector FireDirection)
{
	if (canFire)
	{
		if (FireDirection.SizeSquared() > 0.0f)
		{
			const FRotator FireRotation = FireDirection.Rotation();
			const FVector gunLocation = owner->GetActorLocation() + FireRotation.RotateVector(GunOffset);

			ADroneProjectile* projectile = mSpawnProjectile;
			projectile->SetShooter(owner);
			projectile->SetDamage(FMath::RandRange(damage * 0.95f, damage * 1.05f));
			mSetTimerWolrd(owner->GetWorld(), TimerHandle_ShotTimerExpired, &UWeapon::ShotTimerExpired, fireRate);

			if (FireSound != nullptr)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FireSound, owner->GetActorLocation());
			}

			canFire = false;
		}
	}
}
