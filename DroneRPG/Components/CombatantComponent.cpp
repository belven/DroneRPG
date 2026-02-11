#include "CombatantComponent.h"

UCombatantComponent::UCombatantComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatantComponent::SetupCombatantComponent(FString inName, EDamagerType inType, int32 inTeam)
{
	name = inName;
	type = inType;
	team = inTeam;
}

void UCombatantComponent::UnitKilled(AActor* unitKilled)
{
	OnUnitKilled.Broadcast(unitKilled);
}