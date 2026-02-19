#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <DroneRPG/Utilities/Enums.h>
#include "CombatantComponent.generated.h"

class ADroneRPGGameMode;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUnitKilled, UCombatantComponent*, unitKilled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTeamChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FScoreGained, float, score);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DRONERPG_API UCombatantComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatantComponent();
	FString GetCombatantName() { return name; };
	EDamagerType GetCombatantType() { return type; };
	int32 GetTeam() { return team; };

	void SetTeam(int32 inTeam)
	{
		team = inTeam;
	}

	void SetupCombatantComponent(FString inName, EDamagerType inType);

	UFUNCTION()
	void UnitKilled(UCombatantComponent* unitKilled);

	FUnitKilled OnUnitKilled;
	FTeamChanged OnTeamChanged;
	FScoreGained OnScoreGained;

	ADroneRPGGameMode* GetGameMode() const
	{
		return gameMode;
	}

	void SetGameMode(ADroneRPGGameMode* inGameMode);

	UFUNCTION()
	void AddCombatScore(float score);

	float GetCombatScore() { return combatScore; }
	int32 GetDeaths() const	{		return deaths;	}
	void SetDeaths(int32 inDeaths)	{		deaths = inDeaths;	}
	int32 GetKills() const	{		return kills;	}
	void SetKills(int32 inKills)	{		kills = inKills;	}

	void IncrementDeaths() { deaths++; }
	void IncrementKills() { kills++; }

	FColor GetTeamColour() const
	{
		return teamColour;
	}

	void SetTeamColour(FColor inTeamColour)
	{
		teamColour = inTeamColour;
	}

	UFUNCTION()
	void ResetCombatScore();
private:
	FTimerHandle TimerHandle_ResetCombatScore;

	int32 team;
	FString name;
	EDamagerType type;
	float combatScore;
	int32 kills;
	int32 deaths;
	FColor teamColour;

	UPROPERTY()
	ADroneRPGGameMode* gameMode;
};
