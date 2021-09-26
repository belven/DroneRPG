// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FunctionLibrary.h"
#include "Weapon.generated.h"

#define mSpawnProjectile owner->GetWorld()->SpawnActor<ADroneProjectile>(projectileClass, gunLocation, FireRotation)

class ADroneRPGCharacter;
class ADroneProjectile;

UCLASS()
class DRONERPG_API UWeapon : public UObject
{
	GENERATED_BODY()
public:
	UWeapon();

	 template <class T> static T* CreateWeapon(float inFireRate, float inDamage, ADroneRPGCharacter* inOwner);

	 virtual float GetRange();

	ADroneRPGCharacter* GetOwner() const { return owner; }
	void SetOwner(ADroneRPGCharacter* val) { owner = val; }

	EWeaponType GetWeaponType() const { return weaponType; }
	void SetWeaponType(EWeaponType val) { weaponType = val; }

	virtual void FireShot(FVector FireDirection, AActor* target = NULL);
	virtual ADroneProjectile* SpawnProjectile(FVector gunLocation, FRotator FireRotation, AActor* target);
protected:
	float fireRate;
	float damage;
	bool canFire;
	FVector GunOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* weaponMeshComp;

	UPROPERTY()
		ADroneRPGCharacter* owner;

	UPROPERTY()
		TSubclassOf<ADroneProjectile> projectileClass;

	EWeaponType weaponType;
	class USoundBase* FireSound;
	FTimerHandle TimerHandle_ShotTimerExpired;
	void ShotTimerExpired();
};
