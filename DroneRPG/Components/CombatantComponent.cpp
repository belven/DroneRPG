#include "CombatantComponent.h"

UCombatantComponent::UCombatantComponent(): team(0), type()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatantComponent::SetupCombatantComponent(FString inName, EDamagerType inType)
{
	name = inName;
	type = inType;
}

void UCombatantComponent::UnitKilled(AActor* unitKilled)
{
	OnUnitKilled.Broadcast(unitKilled);
}