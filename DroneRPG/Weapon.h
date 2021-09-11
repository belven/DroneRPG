// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Weapon.generated.h"

class ADroneRPGCharacter;
class ADroneProjectile;

UENUM(BlueprintType)
enum class  EWeaponType : uint8 {
	Laser,
	Rocket,
	Mine,
	Rail_Gun,
	Shotgun
};

UCLASS()
class DRONERPG_API UWeapon : public UObject
{
	GENERATED_BODY()
public:
	UWeapon();

	static UWeapon* CreateWeapon(float inFireRate, float inDamage, ADroneRPGCharacter* inOwner);

	ADroneRPGCharacter* GetOwner() const { return owner; }
	void SetOwner(ADroneRPGCharacter* val) { owner = val; }

	EWeaponType GetWeaponType() const { return weaponType; }
	void SetWeaponType(EWeaponType val) { weaponType = val; }
	void FireShot(FVector FireDirection);

private:
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
