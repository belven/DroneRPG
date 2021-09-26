#pragma once
#include "DominationGameMode.h"
#include "FunctionLibrary.h"
#include <Kismet/KismetSystemLibrary.h>
#include <Kismet/GameplayStatics.h>
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"

ADominationGameMode::ADominationGameMode() : Super()
{
	gameMode = EGameModeType::Domination;
}

void ADominationGameMode::AddTeamScore(int32 team, float bonusScore)
{
	float& score = teamScores.FindOrAdd(team);
	score += bonusScore;

	if (score >= 300) {		
		for (APlayerState* ps : UGameplayStatics::GetGameState(GetWorld())->PlayerArray) {
		APlayerController* con = UGameplayStatics::GetPlayerController(GetWorld(), ps->GetPlayerId());		
		UKismetSystemLibrary::QuitGame(GetWorld(), con, EQuitPreference::Quit, false);			
		}
	}
}

FString ADominationGameMode::GetTeamScoreText(int32 team)
{
	FColor tc = *UFunctionLibrary::GetTeamColours().Find(team);
	float& score = teamScores.FindOrAdd(team);

	TArray< FStringFormatArg > args;
	args.Add(FStringFormatArg(UFunctionLibrary::GetColourString(tc)));
	args.Add(FStringFormatArg((int)FMath::RoundHalfToEven(score)));
	return FString::Format(TEXT("Team {0} has {1} points"), args);
}