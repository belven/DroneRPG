// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Laser.generated.h"

/**
 * 
 */
UCLASS()
class DRONERPG_API ULaser : public UWeapon
{
	GENERATED_BODY()
public:
	ULaser();
	static ULaser* CreateLaser(float inFireRate, float inDamage, ADroneRPGCharacter* inOwner);
	
};
