#pragma once
#include "CoreMinimal.h"
#include "Weapon.h"
#include "Laser.generated.h"

UCLASS()
class DRONERPG_API ULaser : public UWeapon
{
	GENERATED_BODY()
public:
	ULaser();
	static ULaser* CreateLaser(float inFireRate, float inDamage, UCombatantComponent* inOwner);

	template <class T> static T* CreateWeapon(float inFireRate, float inDamage, UCombatantComponent* inOwner);
};