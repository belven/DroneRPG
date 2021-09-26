#pragma once
#include "CoreMinimal.h"
#include "DroneRPGGameMode.h"
#include "DroneDamagerInterface.h"
#include "PlasmaStormGameMode.generated.h"

UCLASS()
class DRONERPG_API APlasmaStormGameMode : public ADroneRPGGameMode
{
	GENERATED_BODY()
public:
	virtual void EntityKilled(AActor* killedEntity, AActor* damager) override;
	float GetKills() const { return kills; }
	void SetKills(float val) { kills = val; }
private:
	float kills;
};