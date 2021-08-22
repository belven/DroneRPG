// Copyright Epic Games, Inc. All Rights Reserved.

#include "DroneRPGGameMode.h"
#include "DroneRPGPlayerController.h"
#include "DroneRPGCharacter.h"
#include "UObject/ConstructorHelpers.h"

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
	//DefaultPawnClass = ADroneRPGCharacter::StaticClass();
}