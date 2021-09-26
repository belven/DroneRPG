// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneRPGGameMode.h"
#include "DroneRPGPlayerController.h"
#include "DroneRPGCharacter.h"
#include "DroneDamagerInterface.h"
#include "UObject/ConstructorHelpers.h"
#include <GameFramework/HUD.h>

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
	// Do Nothing
}

void ADroneRPGGameMode::AddTeamScore(int32 team, float score)
{
	// Do nothing by default
}

FString ADroneRPGGameMode::GetTeamScoreText(int32 team)
{
	return "Not set";
}
