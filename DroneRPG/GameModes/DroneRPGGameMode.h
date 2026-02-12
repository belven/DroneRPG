#pragma once
#include "CoreMinimal.h"
#include "DroneRPG/Utilities/Enums.h"
#include "GameFramework/GameModeBase.h"
#include "DroneRPGGameMode.generated.h"

class ADroneRPGCharacter;

UCLASS(minimalapi)
class ADroneRPGGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADroneRPGGameMode();
	TArray <ADroneRPGCharacter*>& GetDrones() { return  dronesInGame; };

	virtual void BeginPlay() override;
	virtual void EntityKilled(AActor* killedEntity, AActor* damager);

	virtual void AddTeamScore(int32 team, int32 bonusScore);

	virtual FString GetTeamScoreText(int32 team);

	EGameModeType GetGameMode() const { return gameMode; }
	void SetGameMode(EGameModeType val) { gameMode = val; }

	TMap<int32, FTeamScore>& GetTeamScores()
	{
		return teamScores;
	}

protected:
	EGameModeType gameMode;
	TMap<int32, FTeamScore> teamScores;

	UPROPERTY()
	TArray <ADroneRPGCharacter*> dronesInGame;
};