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
	virtual void EntityKilled(AActor* killedEntity, AActor* damager) override;
	float GetKills() const { return kills; }
	void SetKills(float val) { kills = val; }
	virtual TArray<FScoreBoardStat> GetScoreBoardStats() override;
private:
	float kills;
};