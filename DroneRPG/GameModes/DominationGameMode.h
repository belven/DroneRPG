#pragma once
#include "CoreMinimal.h"
#include "DroneRPGGameMode.h"
#include "DominationGameMode.generated.h"

struct FScoreBoardStat;

UCLASS()
class DRONERPG_API ADominationGameMode : public ADroneRPGGameMode
{
	GENERATED_BODY()
public:
	ADominationGameMode();

	virtual void AddTeamScore(int32 team, int32 bonusScore) override;
};