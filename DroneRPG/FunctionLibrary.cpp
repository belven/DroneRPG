#include "FunctionLibrary.h"
#include "Laser.h"
#include "Shotgun.h"
#include "DroneRPGCharacter.h"
#include "RocketLauncher.h"
#include "Weapon.h"

TMap<int32, FColor> UFunctionLibrary::teamColours = GetTeamColours();
TArray <ADroneRPGCharacter*> UFunctionLibrary::dronesInGame = GetDrones();

TArray <ADroneRPGCharacter*> UFunctionLibrary::GetEnemiesInRadius(float radius, const FVector& loc, int32 team)
{
	TArray<ADroneRPGCharacter*> drones;

	for (ADroneRPGCharacter* drone : mGetDronesInRadius(radius, loc))
	{
		// Check if the drone found is an enemy
		if (team != drone->GetTeam()) {
			drones.Add(drone);
		}
	}

	return drones;
}

TArray <ADroneRPGCharacter*> UFunctionLibrary::GetDronesInRadius(float radius, const FVector& loc)
{
	TArray<ADroneRPGCharacter*> dronesInRange;

	for (ADroneRPGCharacter* drone : GetDrones())
	{
		float dist = mDist(drone->GetActorLocation(), loc);

		if (radius == 0 || dist <= radius) {
			dronesInRange.Add(drone);
		}
	}

	return dronesInRange;
}


ADroneRPGCharacter* UFunctionLibrary::GetClosestEnemyInRadius(float radius, const FVector& loc, int32 team)
{
	return mGetClosestActorInArray<ADroneRPGCharacter>(GetEnemiesInRadius(radius, loc, team), loc);
}

TArray <ADroneRPGCharacter*>& UFunctionLibrary::GetDrones()
{
	return dronesInGame;
}

FString UFunctionLibrary::GetColourString(FColor color)
{
	FString colourString = "";

	if (color == FColor::Red) {
		colourString = "Red";
	}
	else if (color == FColor::Green) {
		colourString = "Green";
	}
	else if (color == FColor::Yellow) {
		colourString = "Yellow";
	}
	else if (color == FColor::Silver) {
		colourString = "Silver";
	}
	else if (color == FColor::Black) {
		colourString = "Black";
	}
	else if (color == FColor::Blue) {
		colourString = "Blue";
	}
	else if (color == FColor::Magenta) {
		colourString = "Magenta";
	}
	else if (color == FColor::Cyan) {
		colourString = "Cyan";
	}
	else if (color == FColor::Emerald) {
		colourString = "Emerald";
	}
	else if (color == FColor::Purple) {
		colourString = "Purple";
	}
	else if (color == FColor::Orange) {
		colourString = "Orange";
	}
	else if (color == FColor::Turquoise) {
		colourString = "Turquoise";
	}
	else if (color == FColor::White) {
		colourString = "White";
	}

	return colourString;
}

TMap<int32, FColor>& UFunctionLibrary::GetTeamColours()
{
	if (teamColours.IsEmpty()) {
		teamColours.Add(1, FColor::Blue);
		teamColours.Add(2, FColor::Yellow);
		teamColours.Add(3, FColor::White);
		teamColours.Add(4, FColor::Magenta);
	}
	return teamColours;
}

UWeapon* UFunctionLibrary::GetDefaultWeapon(EWeaponType type, ADroneRPGCharacter* inOwner)
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
	default:
		break;
	}
	return ULaser::CreateLaser(0.3f, 20.0f, inOwner);
}