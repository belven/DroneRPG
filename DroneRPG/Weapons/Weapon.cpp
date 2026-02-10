#pragma once
#include "Weapon.h"
#include "DroneProjectile.h"
#include "Laser.h"
#include "RocketLauncher.h"
#include "Shotgun.h"
#include "DroneRPG/DroneRPGCharacter.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"

UWeapon::UWeapon() : weaponMeshComp(nullptr), owner(nullptr), FireSound(nullptr)
{
	fireRate = 0.3f;
	damage = 15.0f;
	canFire = true;
	weaponType = EWeaponType::Laser;
	GunOffset = FVector(100.f, 0.f, 0.f);

	static ConstructorHelpers::FClassFinder<ADroneProjectile> ProjectileClassFound(TEXT("/Game/TopDownCPP/Blueprints/Projectiles/Base"));

	if (ProjectileClassFound.Succeeded())
	{
		projectileClass = ProjectileClassFound.Class;
	}
}

template<class T>
T* UWeapon::CreateWeapon(float inFireRate, float inDamage, ADroneRPGCharacter* inOwner)
{
	T* weapon = NewObject<T>(T::StaticClass());

	weapon->fireRate = inFireRate;
	weapon->damage = inDamage;
	weapon->owner = inOwner;

	return weapon;
}

float UWeapon::GetRange()
{
	return ADroneProjectile::Default_Initial_Speed * ADroneProjectile::Default_Initial_Lifespan;
}

void UWeapon::ShotTimerExpired()
{
	canFire = true;
}

ADroneProjectile* UWeapon::SpawnProjectile(FVector gunLocation, FRotator FireRotation) {
	ADroneProjectile* projectile = mSpawnProjectile;
	projectile->SetShooter(owner);
	projectile->SetDamage(FMath::RandRange(damage * 0.95f, damage * 1.05f));
	return projectile;
}

UWeapon* UWeapon::GetWeapon(EWeaponType type, float inFireRate, float inDamage, ADroneRPGCharacter* inOwner)
{
	switch (type) {
	case EWeaponType::Laser:
		return ULaser::CreateLaser(0.3f, 20.0f, inOwner);
	case EWeaponType::Rocket:
		return URocketLauncher::CreateRocketLauncher(1.5f, 60.0f, inOwner);
	case EWeaponType::Mine:
		return UShotgun::CreateShotgun(0.5f, 15.0f, inOwner);
	case EWeaponType::Rail_Gun:
		return ULaser::CreateLaser(0.3f, 20.0f, inOwner);
	case EWeaponType::Shotgun:
		return UShotgun::CreateShotgun(0.5f, 15.0f, inOwner);
	default:
		break;
	}
	return ULaser::CreateLaser(0.3f, 20.0f, inOwner);
}

UWeapon* UWeapon::GetDefaultWeapon(EWeaponType type, ADroneRPGCharacter* inOwner)
{
	switch (type) {
	case EWeaponType::Rail_Gun:
	case EWeaponType::Laser:
		return ULaser::CreateLaser(0.3f, 25.0f, inOwner);
	case EWeaponType::Rocket:
		return URocketLauncher::CreateRocketLauncher(1.5f, 50.0f, inOwner);
	case EWeaponType::Mine:
	case EWeaponType::Shotgun:
		return UShotgun::CreateShotgun(0.5f, 15.0f, inOwner);
	default:
		break;
	}
	return ULaser::CreateLaser(0.3f, 20.0f, inOwner);
}

void UWeapon::FireShot(FVector FireDirection)
{
	if (canFire)
	{
		if (FireDirection.SizeSquared() > 0.0f)
		{
			const FRotator FireRotation = FireDirection.Rotation();
			const FVector gunLocation = owner->GetActorLocation() + FireRotation.RotateVector(GunOffset);
			SpawnProjectile(gunLocation, FireRotation);

			mSetTimerWorld(owner->GetWorld(), TimerHandle_ShotTimerExpired, &UWeapon::ShotTimerExpired, fireRate);

			if (FireSound != nullptr)
			{
				//UGameplayStatics::PlaySoundAtLocation(this, FireSound, owner->GetActorLocation());
			}

			canFire = false;
		}
	}
}
