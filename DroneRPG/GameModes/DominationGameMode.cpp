#pragma once
#include "DominationGameMode.h"
#include <Kismet/KismetSystemLibrary.h>
#include <Kismet/GameplayStatics.h>
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"

ADominationGameMode::ADominationGameMode()
{
	gameMode = EGameModeType::Domination;
}

void ADominationGameMode::AddTeamScore(int32 team, int32 bonusScore)
{
	Super::AddTeamScore(team, bonusScore);

	int32& score = teamScores.FindOrAdd(team).score;

	if (score >= 1000) 
	{
		for (APlayerState* ps : UGameplayStatics::GetGameState(GetWorld())->PlayerArray) 
		{
			APlayerController* con = UGameplayStatics::GetPlayerController(GetWorld(), ps->GetPlayerId());
			UKismetSystemLibrary::QuitGame(GetWorld(), con, EQuitPreference::Quit, false);
		}
	}
}