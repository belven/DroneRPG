#pragma once
#include "RocketLauncher.h"
#include <DroneRPG/Components/CombatantComponent.h>
#include "DroneProjectile.h"
#include "Rocket.h"

URocketLauncher::URocketLauncher()
{
	weaponType = EWeaponType::Rocket;
	projectileSpeed = ARocket::Default_Initial_Speed;
	lifespan = ARocket::Default_Initial_Lifespan;

	static ConstructorHelpers::FClassFinder<ADroneProjectile> rocketProjectile(TEXT("/Game/TopDownCPP/Blueprints/Projectiles/Rocket"));

	if (rocketProjectile.Succeeded()) 
	{
		projectileClass = rocketProjectile.Class;
	}
}

URocketLauncher* URocketLauncher::CreateRocketLauncher(float inFireRate, float inDamage, UCombatantComponent* inOwner)
{
	URocketLauncher* weapon = NewObject<URocketLauncher>(StaticClass());
	weapon->fireRate = inFireRate;
	weapon->damage = inDamage;
	weapon->owner = inOwner;
	return weapon;
}

ADroneProjectile* URocketLauncher::SpawnProjectile(FVector gunLocation, FRotator FireRotation)
{
	return Super::SpawnProjectile(gunLocation, FireRotation);
}
