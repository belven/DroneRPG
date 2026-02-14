#pragma once
#include "CoreMinimal.h"
#include "Enums.generated.h"

USTRUCT(BlueprintType)
struct FTeamScore
{
	GENERATED_USTRUCT_BODY()
	FTeamScore() : team(0), teamColour(), score(0)
	{
		teamName = FString::FromInt(team);
	}

	FTeamScore(int32 inTeam, const FColor& inTeamColour, int32 inScore, FString inTeamName = "")
		: team(inTeam),
		teamColour(inTeamColour),
		score(inScore)
	{
		if (inTeamName.IsEmpty()) {
			teamName = FString::FromInt(team);
		} else
		{
			teamName = inTeamName;
		}
	}

	FString GetTeamScoreText()
	{
		TArray< FStringFormatArg > args;
		args.Add(FStringFormatArg(teamName));
		args.Add(FStringFormatArg(score));
		return FString::Format(TEXT("Team {0} has {1} points"), args);
	}

	FString teamName;
	int32 team;
	FColor teamColour;
	int32 score;
};

UENUM(BlueprintType)
enum class  EWeaponType : uint8 {
	Laser,
	Rocket,
	Mine,
	Rail_Gun,
	Shotgun,
	End
};

UENUM(BlueprintType)
enum class  EActionState : uint8 {
	MovingToObjective,
	AttackingTarget,
	CapturingObjective,
	DefendingObjective,
	ReturningToBase,
	Start
};

UENUM(BlueprintType)
enum class  EGameModeType : uint8 {
	Domination,
	TeamDeathMatch,
	PlasmaStorm,
	Hardpoint,
	AttackDefend,
	Payload
};

UENUM(BlueprintType)
enum class  EDamagerType : uint8 {
	Drone,
	PlasmaStorm
};

UCLASS()
class DRONERPG_API UEnums : public UObject
{
	GENERATED_BODY()
};