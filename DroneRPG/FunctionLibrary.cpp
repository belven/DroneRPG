#include "FunctionLibrary.h"
#include "Laser.h"
#include "Shotgun.h"
#include "DroneRPGCharacter.h"
#include "Objective.h"
#include "RocketLauncher.h"

TMap<int32, FColor> UFunctionLibrary::teamColours = UFunctionLibrary::GetTeamColours();

TArray <ADroneRPGCharacter*> UFunctionLibrary::GetEnemysInRadius(UWorld* world, float radius, FVector loc, int32 team)
{
	TArray<ADroneRPGCharacter*> drones;

	for (ADroneRPGCharacter* drone : mGetActorsInRadius<ADroneRPGCharacter>(world, radius, loc))
	{
		// Check if the drone found is an enemy
		if (team != drone->GetTeam()) {
			drones.Add(drone);
		}
	}

	return drones;
}

ADroneRPGCharacter* UFunctionLibrary::GetClosestEnemyInRadius(UWorld* world, float radius, FVector loc, int32 team)
{
	return mGetClosestActorInArray<ADroneRPGCharacter>(mGetEnemysInRadius(world, radius, loc, team), loc);
}

TMap<int32, FColor> UFunctionLibrary::GetTeamColours()
{
	if (UFunctionLibrary::teamColours.IsEmpty()) {
		teamColours.Add(1, FColor::Blue);
		teamColours.Add(2, FColor::Yellow);
		teamColours.Add(3, FColor::Silver);
		teamColours.Add(4, FColor::Magenta);
	}
	return teamColours;
}

UWeapon* UFunctionLibrary::GetWeapon(EWeaponType type, float inFireRate, float inDamage, ADroneRPGCharacter* inOwner)
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
		break;
	}
	return ULaser::CreateLaser(0.3f, 20.0f, inOwner);
}