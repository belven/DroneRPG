#pragma once
#include "CoreMinimal.h"
#include "Weapon.h"
#include "RocketLauncher.generated.h"

UCLASS()
class DRONERPG_API URocketLauncher : public UWeapon
{
	GENERATED_BODY()
public:
	URocketLauncher();
	static URocketLauncher* CreateRocketLauncher(float inFireRate, float inDamage, ADroneRPGCharacter* inOwner);
	virtual ADroneProjectile* SpawnProjectile(FVector gunLocation, FRotator FireRotation) override;
};