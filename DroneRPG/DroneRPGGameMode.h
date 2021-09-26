#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Enums.h"
#include "DroneRPGGameMode.generated.h"

UCLASS(minimalapi)
class ADroneRPGGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADroneRPGGameMode();

	virtual void BeginPlay() override;
	virtual void EntityKilled(AActor* killedEntity, AActor* damager);

	virtual TArray<FScoreBoardStat> GetScoreBoardStats();

	virtual void AddTeamScore(int32 team, float bonusScore);

	virtual FString GetTeamScoreText(int32 team);

	EGameModeType GetGameMode() const { return gameMode; }
	void SetGameMode(EGameModeType val) { gameMode = val; }

protected:
	EGameModeType gameMode;
	TMap<int32, float> teamScores;
};