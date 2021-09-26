// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneRPGGameMode.h"
#include "DroneRPGPlayerController.h"
#include "DroneRPGCharacter.h"
#include "DroneDamagerInterface.h"
#include "UObject/ConstructorHelpers.h"
#include <GameFramework/HUD.h>
#include "FunctionLibrary.h"

ADroneRPGGameMode::ADroneRPGGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ADroneRPGPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/DroneCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	
	static ConstructorHelpers::FClassFinder<AHUD> hud(TEXT("Blueprint'/Game/TopDownCPP/Blueprints/BaseHud.BaseHud_C'"));
	HUDClass = hud.Class;
}

void ADroneRPGGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ADroneRPGGameMode::EntityKilled(AActor* killedEntity, AActor* damager)
{
	if (mImplements(damager, UDroneDamagerInterface)) {
		IDroneDamagerInterface* damageDealer = Cast<IDroneDamagerInterface>(damager);
		float& score = teamScores.FindOrAdd(damageDealer->GetDamagerTeam());
		score++;
	}
}

TArray<FScoreBoardStat> ADroneRPGGameMode::GetScoreBoardStats()
{
	TArray<FScoreBoardStat> stats;
	return stats;
}

void ADroneRPGGameMode::AddTeamScore(int32 team, float bonusScore)
{
	float& score = teamScores.FindOrAdd(team);
	score += bonusScore;
}

FString ADroneRPGGameMode::GetTeamScoreText(int32 team)
{
	FColor tc = *UFunctionLibrary::GetTeamColours().Find(team);
	float& score = teamScores.FindOrAdd(team);

	TArray< FStringFormatArg > args;
	args.Add(FStringFormatArg(UFunctionLibrary::GetColourString(tc)));
	args.Add(FStringFormatArg((int)FMath::RoundHalfToEven(score)));
	return FString::Format(TEXT("Team {0} has {1} points"), args);
}
