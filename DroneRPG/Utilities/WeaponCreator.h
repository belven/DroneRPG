#pragma once
#include "CoreMinimal.h"
#include <DroneRPG/Weapons/Weapon.h>
#include "WeaponCreator.generated.h"

#define mGetWeapon UWeaponCreator::GetWeapon
#define mGetDefaultWeapon UWeaponCreator::GetDefaultWeapon

UCLASS()
class DRONERPG_API UWeaponCreator : public UObject
{
	GENERATED_BODY()
public:

	static UWeapon* GetDefaultWeapon(EWeaponType type, UCombatantComponent* inOwner);
	static UWeapon* GetWeapon(EWeaponType type, float inFireRate, float inDamage, UCombatantComponent* inOwner);
};
