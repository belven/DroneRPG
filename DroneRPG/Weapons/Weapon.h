#pragma once
#include "CoreMinimal.h"
#include "DroneRPG/Enums.h"
#include "Weapon.generated.h"

#define mSpawnProjectile owner->GetWorld()->SpawnActor<ADroneProjectile>(projectileClass, gunLocation, FireRotation)
#define mGetWeapon UWeapon::GetWeapon
#define mGetDefaultWeapon UWeapon::GetDefaultWeapon

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

	static UWeapon* GetDefaultWeapon(EWeaponType type, ADroneRPGCharacter* inOwner);
	static UWeapon* GetWeapon(EWeaponType type, float inFireRate, float inDamage, ADroneRPGCharacter* inOwner);
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

	UPROPERTY()
	USoundBase* FireSound;
	FTimerHandle TimerHandle_ShotTimerExpired;
	void ShotTimerExpired();
};
