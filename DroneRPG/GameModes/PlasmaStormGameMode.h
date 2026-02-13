#pragma once
#include "CoreMinimal.h"
#include "DroneRPGGameMode.h"
#include "PlasmaStormGameMode.generated.h"

struct FScoreBoardStat;

UCLASS()
class DRONERPG_API APlasmaStormGameMode : public ADroneRPGGameMode
{
	GENERATED_BODY()
public:
	APlasmaStormGameMode();
	virtual void BeginPlay() override;
	virtual void EntityKilled(UCombatantComponent* killedEntity, UCombatantComponent* damager) override;
	float GetKills() const { return kills; }
	void SetKills(float val) { kills = val; }
private:
	float kills;
};