#pragma once
#include "Laser.h"

ULaser::ULaser()
{
	weaponType = EWeaponType::Laser;
}

ULaser* ULaser::CreateLaser(float inFireRate, float inDamage, ADroneRPGCharacter* inOwner)
{
	ULaser* weapon = NewObject<ULaser>(StaticClass());
	weapon->fireRate = inFireRate;
	weapon->damage = inDamage;
	weapon->owner = inOwner;
	return weapon;
}

template <class T>
T* ULaser::CreateWeapon(float inFireRate, float inDamage, ADroneRPGCharacter* inOwner)
{
	ULaser* weapon = NewObject<ULaser>(StaticClass());
	weapon->fireRate = inFireRate;
	weapon->damage = inDamage;
	weapon->owner = inOwner;
	return weapon;
}