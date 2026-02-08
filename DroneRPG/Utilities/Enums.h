#pragma once
#include "CoreMinimal.h"
#include "Enums.generated.h"

USTRUCT(BlueprintType)
struct FScoreBoardStat
{
	GENERATED_USTRUCT_BODY()

	FScoreBoardStat(): textColour()
	{
	}

	FScoreBoardStat(FString inText, FColor inColour) {
		text = inText;
		textColour = inColour;
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FString text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FColor textColour;
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
	SearchingForObjective,
	AttackingTarget,
	CapturingObjective,
	DefendingObjective,
	ReturningToBase
};

UENUM(BlueprintType)
enum class  EGameModeType : uint8 {
	Domination,
	TeamDeathMatch,
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