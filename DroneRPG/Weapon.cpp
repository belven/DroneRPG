#pragma once
#include "Weapon.h"
#include "DroneProjectile.h"
#include "DroneRPGCharacter.h"
#include "FunctionLibrary.h"
#include "Enums.h"

UWeapon::UWeapon(): weaponMeshComp(nullptr), owner(nullptr), FireSound(nullptr)
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

ADroneProjectile* UWeapon::SpawnProjectile(FVector gunLocation, FRotator FireRotation, AActor* target) {
	ADroneProjectile* projectile = mSpawnProjectile;
	projectile->SetShooter(owner);
	projectile->SetDamage(FMath::RandRange(damage * 0.95f, damage * 1.05f));
	return projectile;
}

void UWeapon::FireShot(FVector FireDirection, AActor* target)
{
	if (canFire)
	{
		if (FireDirection.SizeSquared() > 0.0f)
		{
			const FRotator FireRotation = FireDirection.Rotation();
			const FVector gunLocation = owner->GetActorLocation() + FireRotation.RotateVector(GunOffset);
			SpawnProjectile(gunLocation, FireRotation, target);

			mSetTimerWorld(owner->GetWorld(), TimerHandle_ShotTimerExpired, &UWeapon::ShotTimerExpired, fireRate);

			if (FireSound != nullptr)
			{
				//UGameplayStatics::PlaySoundAtLocation(this, FireSound, owner->GetActorLocation());
			}

			canFire = false;
		}
	}
}