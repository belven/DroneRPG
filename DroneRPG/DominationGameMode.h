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

	virtual void AddTeamScore(int32 team, float bonusScore) override;
	virtual TArray<FScoreBoardStat> GetScoreBoardStats() override;

	TMap<int32, float> GetTeamScores() const { return teamScores; }
	void SetTeamScores(TMap<int32, float> val) { teamScores = val; }
};