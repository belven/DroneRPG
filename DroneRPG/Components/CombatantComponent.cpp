#include "CombatantComponent.h"

#include "DroneRPG/GameModes/DroneRPGGameMode.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"

UCombatantComponent::UCombatantComponent(): team(0), type(), combatScore(0), gameMode(nullptr)
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

void UCombatantComponent::SetGameMode(ADroneRPGGameMode* inGameMode)
{
	gameMode = inGameMode;
	gameMode->GetCombatants().AddUnique(this);
}

void UCombatantComponent::AddCombatScore(float score)
{
	combatScore += score;
	OnScoreGained.Broadcast(score);

	if (IsValid(GetWorld()))
	{
		mSetTimer(TimerHandle_ResetCombatScore, &UCombatantComponent::ResetCombatScore, 2.0f);
	}
}

void UCombatantComponent::ResetCombatScore()
{
	combatScore = 0;
}
