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
	virtual float GetRange() override;
	
	static UShotgun* CreateShotgun(float inFireRate, float inDamage, ADroneRPGCharacter* inOwner, int32 inPellets = 4);

	FRotator RandomDirection(FRotator fireRotation);
	virtual void FireShot(FVector FireDirection, AActor* target) override;
	virtual ADroneProjectile* SpawnProjectile(FVector gunLocation, FRotator FireRotation, AActor* target) override;
private:
	int32 pellets;
	float spread;
};