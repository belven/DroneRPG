#pragma once
#include "RocketLauncher.h"
#include "DroneProjectile.h"
#include "Rocket.h"
#include "DroneRPGCharacter.h"
#include "Enums.h"

URocketLauncher::URocketLauncher() : Super()
{
	weaponType = EWeaponType::Rocket;

	static ConstructorHelpers::FClassFinder<ADroneProjectile> rocketProjectile(TEXT("/Game/TopDownCPP/Blueprints/Projectiles/Rocket"));

	if (rocketProjectile.Succeeded()) {
		projectileClass = rocketProjectile.Class;
	}
}

float URocketLauncher::GetRange()
{
	return ARocket::Default_Initial_Speed * ARocket::Default_Initial_Lifespan;
}

URocketLauncher* URocketLauncher::CreateRocketLauncher(float inFireRate, float inDamage, ADroneRPGCharacter* inOwner)
{
	URocketLauncher* weapon = NewObject<URocketLauncher>(URocketLauncher::StaticClass());
	weapon->fireRate = inFireRate;
	weapon->damage = inDamage;
	weapon->owner = inOwner;
	return weapon;
}

ADroneProjectile* URocketLauncher::SpawnProjectile(FVector gunLocation, FRotator FireRotation, AActor* target)
{
	ARocket* proj = Cast<ARocket>(Super::SpawnProjectile(gunLocation, FireRotation, target));
	ADroneRPGCharacter* targetEnemy = Cast<ADroneRPGCharacter>(target);

	if (proj != NULL && targetEnemy != NULL) {
		proj->SetTarget(targetEnemy);
	}
	return proj;
}
