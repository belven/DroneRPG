#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FunctionLibrary.h"
#include "DroneDamagerInterface.h"
#include "DroneRPGGameMode.generated.h"

UCLASS(minimalapi)
class ADroneRPGGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADroneRPGGameMode();

	virtual void BeginPlay() override;
	virtual void EntityKilled(AActor* killedEntity, AActor* damager);

	virtual void AddTeamScore(int32 team, float score);

	virtual FString GetTeamScoreText(int32 team);

	EGameModeType GetGameMode() const { return gameMode; }
	void SetGameMode(EGameModeType val) { gameMode = val; }

protected:
	EGameModeType gameMode;
};