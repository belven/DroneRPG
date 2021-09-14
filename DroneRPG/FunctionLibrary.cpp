#include "FunctionLibrary.h"
#include "Laser.h"
#include "Shotgun.h"
#include "DroneRPGCharacter.h"
#include "Objective.h"

TMap<int32, FColor> UFunctionLibrary::teamColours = UFunctionLibrary::GetTeamColours();

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
		return ULaser::CreateLaser(inFireRate, inDamage, inOwner);
	case EWeaponType::Rocket:
		return UShotgun::CreateShotgun(inFireRate, inDamage, inOwner);
	case EWeaponType::Mine:
		return UShotgun::CreateShotgun(inFireRate, inDamage, inOwner);
	case EWeaponType::Rail_Gun:
		return ULaser::CreateLaser(inFireRate, inDamage, inOwner);
	case EWeaponType::Shotgun:
		return UShotgun::CreateShotgun(inFireRate, inDamage, inOwner);
		break;
	}
	return ULaser::CreateLaser(inFireRate, inDamage, inOwner);
}