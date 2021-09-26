#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DroneRPGGameMode.h"
#include "Enums.h"
#include "DroneDamagerInterface.generated.h"

class ADroneRPGCharacter;

UINTERFACE(MinimalAPI)
class UDroneDamagerInterface : public UInterface
{
	GENERATED_BODY()
};

class DRONERPG_API IDroneDamagerInterface
{
	GENERATED_BODY()
public:
	virtual void DroneKilled(ADroneRPGCharacter* drone);
	virtual FString GetDamagerName();
	virtual EDamagerType GetDamagerType();
	virtual int32 GetDamagerTeam();
};