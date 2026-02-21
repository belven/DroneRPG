#pragma once
#include "CoreMinimal.h"
#include "DroneRPG/Utilities/Enums.h"
#include "GameFramework/GameModeBase.h"
#include "DroneRPGGameMode.generated.h"

class UObjectiveComponent;
class UCombatantComponent;
class ADroneRPGCharacter;

UCLASS(minimalapi)
class ADroneRPGGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADroneRPGGameMode();
	void UnitHit(float inDamage, UCombatantComponent* attacker);

	void CleanUp();
	virtual void BeginPlay() override;
	virtual void EntityKilled(UCombatantComponent* killedEntity, UCombatantComponent* killer);

	virtual void AddTeamScore(int32 team, int32 bonusScore);
	void SortTeams();

	virtual FString GetTeamScoreText(int32 team);

	EGameModeType GetGameMode() const { return gameMode; }
	void SetGameMode(EGameModeType val) { gameMode = val; }

	FColor GetTeamColour(int32 team);
	TMap<int32, FColor>& GetTeamColours();

	TMap<int32, FTeamScore>& GetTeamScores()
	{
		return teamScores;
	}

	TArray<UCombatantComponent*>& GetCombatants()
	{
		return combatants;
	}

	void AddCombatant(UCombatantComponent* combatant);

	TArray<::UObjectiveComponent*>& GetObjectives()
	{
		return objectives;
	}

	void AddObjective(UObjectiveComponent* newObjective);
protected:
	FTeamScore topTeam;
	EGameModeType gameMode;
	TMap<int32, FTeamScore> teamScores;
	TMap<int32, FColor> teamColours;
	TArray<FColor> colours;
	bool coloursSet;
	FTimerHandle TimerHandle_CleanUp;

	FTimerHandle TimerHandle_SortTeamsTimer;

	UPROPERTY()
	TArray <UCombatantComponent*> combatants;

	UPROPERTY()
	TArray <UObjectiveComponent*> objectives;
};