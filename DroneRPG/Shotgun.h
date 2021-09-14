// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class DRONERPG_API UShotgun : public UWeapon
{
	GENERATED_BODY()
public:
	UShotgun();
	
	static UShotgun* CreateShotgun(float inFireRate, float inDamage, ADroneRPGCharacter* inOwner);

	FRotator RandomDirection(FRotator fireRotation);
	virtual void FireShot(FVector FireDirection) override;
};
