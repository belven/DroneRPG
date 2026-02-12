#pragma once
#include "CoreMinimal.h"
#include "DroneRPG/Utilities/Enums.h"
#include "Weapon.generated.h"

#define mSpawnProjectile owner->GetWorld()->SpawnActor<ADroneProjectile>(projectileClass, gunLocation, FireRotation)
#define mGetWeapon UWeapon::GetWeapon
#define mGetDefaultWeapon UWeapon::GetDefaultWeapon

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

	virtual void FireShot(FVector FireDirection);
	virtual ADroneProjectile* SpawnProjectile(FVector gunLocation, FRotator FireRotation);

	static UWeapon* GetDefaultWeapon(EWeaponType type, UCombatantComponent* inOwner);
	static UWeapon* GetWeapon(EWeaponType type, float inFireRate, float inDamage, UCombatantComponent* inOwner);
protected:
	float fireRate;
	float damage;
	float lifespan;
	float speed;
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
