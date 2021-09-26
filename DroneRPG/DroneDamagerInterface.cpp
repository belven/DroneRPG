#include "DroneDamagerInterface.h"
#include "Enums.h"

void IDroneDamagerInterface::DroneKilled(ADroneRPGCharacter* drone)
{
	// Do nothing
}

FString IDroneDamagerInterface::GetDamagerName()
{
	return "";
}

EDamagerType IDroneDamagerInterface::GetDamagerType()
{
	return EDamagerType::Drone;
}

int32 IDroneDamagerInterface::GetDamagerTeam()
{
	return 0;
}
