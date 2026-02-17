#pragma once
#include "CoreMinimal.h"
#include "Weapon.h"
#include "Shotgun.generated.h"

UCLASS()
class DRONERPG_API UShotgun : public UWeapon
{
	GENERATED_BODY()
public:
	static const float Default_Initial_Lifespan;

	UShotgun();
	
	static UShotgun* CreateShotgun(float inFireRate, float inDamage, UCombatantComponent* inOwner, int32 inPellets = 4);

	FRotator RandomDirection(FRotator fireRotation);
	virtual void FireShot() override;
private:
	int32 pellets;
	float spread;
};