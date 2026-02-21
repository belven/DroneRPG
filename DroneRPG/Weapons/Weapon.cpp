#pragma once
#include "Weapon.h"
#include <DroneRPG/Components/CombatantComponent.h>
#include "DroneProjectile.h"
#include "DroneRPG/DroneRPGCharacter.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"

UWeapon::UWeapon() :
	lifespan(ADroneProjectile::Default_Initial_Lifespan),
	projectileSpeed(ADroneProjectile::Default_Initial_Speed),
	weaponMeshComp(nullptr),
	owner(nullptr),
	FireSound(nullptr),
	isActive(false)
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
T* UWeapon::CreateWeapon(float inFireRate, float inDamage, UCombatantComponent* inOwner)
{
	T* weapon = NewObject<T>(T::StaticClass());
	weapon->fireRate = inFireRate;
	weapon->damage = inDamage;
	weapon->owner = inOwner;
	return weapon;
}

float UWeapon::GetRange()
{
	return  projectileSpeed * lifespan * .5;
}

void UWeapon::ShotTimerExpired()
{
	canFire = true;

	if (isActive)
	{
		FireShot();
	}
}

ADroneProjectile* UWeapon::SpawnProjectile(FVector gunLocation, FRotator FireRotation) {
	ADroneProjectile* projectile = mSpawnProjectile;
	projectile->SetShooter(GetOwner());
	//projectile->SetDamage(FMath::RandRange(damage * 0.95f, damage * 1.05f)); TODO Do we want random damage?
	projectile->SetDamage(damage);
	projectile->SetLifeSpan(lifespan);
	projectile->GetProjectileMovement()->MaxSpeed = GetProjectileSpeed();
	projectile->GetProjectileMovement()->InitialSpeed = GetProjectileSpeed();
	return projectile;
}

void UWeapon::SetActive(bool inActiveState)
{
	if (!isActive && inActiveState)
	{
		FireShot();
	}

	isActive = inActiveState;
}

UE::Math::TVector<double> UWeapon::GetFireDirection()
{
	return GetOwner()->GetOwner()->GetActorRotation().Vector();
}

void UWeapon::FireShot()
{
	if (canFire)
	{
		const FVector FireDirection = GetFireDirection().GetSafeNormal();

		if (!FireDirection.IsNearlyZero())
		{
			const FRotator fireRotation = FireDirection.ToOrientationRotator();
			const FTransform& ownerTransform = owner->GetOwner()->GetActorTransform();
			const FVector gunLocation = ownerTransform.TransformPosition(GunOffset);

			ADroneProjectile* projectile = SpawnProjectile(gunLocation, fireRotation);		
			projectile->GetProjectileMovement()->Velocity = FireDirection * GetProjectileSpeed();
			mSetTimerWorld(owner->GetWorld(), TimerHandle_ShotTimerExpired, &UWeapon::ShotTimerExpired, fireRate);

			if (FireSound != nullptr)
			{
				//UGameplayStatics::PlaySoundAtLocation(this, FireSound, owner->GetActorLocation());
			}

			canFire = false;
		}
	}
}
