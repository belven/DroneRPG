#include "WeaponCreator.h"
#include <DroneRPG/Weapons/Shotgun.h>
#include <DroneRPG/Weapons/Laser.h>
#include <DroneRPG/Weapons/RocketLauncher.h>

UWeapon* UWeaponCreator::GetWeapon(EWeaponType type, float inFireRate, float inDamage, UCombatantComponent* inOwner)
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

UWeapon* UWeaponCreator::GetDefaultWeapon(EWeaponType type, UCombatantComponent* inOwner)
{
	switch (type) {
	case EWeaponType::Rail_Gun:
	case EWeaponType::Laser:
		return ULaser::CreateLaser(0.3f, 35.0f, inOwner);
	case EWeaponType::Rocket:
		return URocketLauncher::CreateRocketLauncher(1.5f, 75.0f, inOwner);
	case EWeaponType::Mine:
	case EWeaponType::Shotgun:
		return UShotgun::CreateShotgun(0.5f, 25.0f, inOwner);
	default:
		break;
	}
	return ULaser::CreateLaser(0.3f, 20.0f, inOwner);
}