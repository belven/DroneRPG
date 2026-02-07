#pragma once
#include "DominationGameMode.h"
#include "FunctionLibrary.h"
#include <Kismet/KismetSystemLibrary.h>
#include <Kismet/GameplayStatics.h>
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"

ADominationGameMode::ADominationGameMode()
{
	gameMode = EGameModeType::Domination;
}

void ADominationGameMode::AddTeamScore(int32 team, float bonusScore)
{
	Super::AddTeamScore(team, bonusScore);

	float& score = teamScores.FindOrAdd(team);
	if (score >= 1000) {		
		for (APlayerState* ps : UGameplayStatics::GetGameState(GetWorld())->PlayerArray) {
		APlayerController* con = UGameplayStatics::GetPlayerController(GetWorld(), ps->GetPlayerId());		
		UKismetSystemLibrary::QuitGame(GetWorld(), con, EQuitPreference::Quit, false);			
		}
	}
}

TArray<FScoreBoardStat> ADominationGameMode::GetScoreBoardStats()
{
	TArray<FScoreBoardStat> stats; 

	// Domination just uses the base score of the teams
	for (auto& team : UFunctionLibrary::GetTeamColours()) {
		FScoreBoardStat stat;
		stat.text = GetTeamScoreText(team.Key);
		stat.textColour = team.Value;
		stats.Add(stat);
	}

	return stats;
}