#pragma once
#include "CoreMinimal.h"
#include "DroneRPG/Utilities/Enums.h"
#include "Weapon.generated.h"

#define mSpawnProjectile owner->GetWorld()->SpawnActor<ADroneProjectile>(projectileClass, gunLocation, FireRotation)

class UCombatantComponent;
class ADroneRPGCharacter;
class ADroneProjectile;

UCLASS()
class DRONERPG_API UWeapon : public UObject
{
	GENERATED_BODY()
public:
	UWeapon();

	template <class T> static T* CreateWeapon(float inFireRate, float inDamage, UCombatantComponent* inOwner);

	virtual float GetRange();

	UCombatantComponent* GetOwner() const { return owner; }
	void SetOwner(UCombatantComponent* val) { owner = val; }

	EWeaponType GetWeaponType() const { return weaponType; }
	void SetWeaponType(EWeaponType val) { weaponType = val; }

	float GetProjectileSpeed() const { return projectileSpeed; }
	void SetProjectileSpeed(float inProjectileSpeed) { projectileSpeed = inProjectileSpeed; }

	virtual void FireShot(FVector FireDirection);
	virtual ADroneProjectile* SpawnProjectile(FVector gunLocation, FRotator FireRotation);
protected:
	float fireRate;
	float damage;
	float lifespan;
	float projectileSpeed;
	bool canFire;
	FVector GunOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* weaponMeshComp;

	UPROPERTY()
	UCombatantComponent* owner;

	UPROPERTY()
	TSubclassOf<ADroneProjectile> projectileClass;

	EWeaponType weaponType;

	UPROPERTY()
	USoundBase* FireSound;
	FTimerHandle TimerHandle_ShotTimerExpired;
	void ShotTimerExpired();
};
