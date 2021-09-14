// Fill out your copyright notice in the Description page of Project Settings.


#include "Laser.h"

ULaser::ULaser() : Super()
{
	weaponType = EWeaponType::Laser;
}

ULaser* ULaser::CreateLaser(float inFireRate, float inDamage, ADroneRPGCharacter* inOwner)
{
	ULaser* weapon = NewObject<ULaser>(ULaser::StaticClass());
	weapon->fireRate = inFireRate;
	weapon->damage = inDamage;
	weapon->owner = inOwner;
	return weapon;
}
