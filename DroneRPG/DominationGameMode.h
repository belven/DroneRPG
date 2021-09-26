#pragma once
#include "CoreMinimal.h"
#include "DroneRPGGameMode.h"
#include "Enums.h"
#include "DominationGameMode.generated.h"

UCLASS()
class DRONERPG_API ADominationGameMode : public ADroneRPGGameMode
{
	GENERATED_BODY()
public:
	ADominationGameMode();

	virtual void AddTeamScore(int32 team, float score) override;
	virtual FString GetTeamScoreText(int32 team) override;

	TMap<int32, float> GetTeamScores() const { return teamScores; }
	void SetTeamScores(TMap<int32, float> val) { teamScores = val; }
protected:
	TMap<int32, float> teamScores;
};