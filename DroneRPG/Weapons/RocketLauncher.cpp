#pragma once
#include "RocketLauncher.h"

#include <DroneRPG/Components/CombatantComponent.h>

#include "DroneProjectile.h"
#include "Rocket.h"
#include "DroneRPG/DroneRPGCharacter.h"

URocketLauncher::URocketLauncher()
{
	weaponType = EWeaponType::Rocket;
	speed = ARocket::Default_Initial_Speed;
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
	ARocket* proj = Cast<ARocket>(Super::SpawnProjectile(gunLocation, FireRotation));
	proj->team = GetOwner()->GetTeam();
	return proj;
}
