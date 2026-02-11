#include "DroneRPGGameMode.h"
#include <DroneRPG/Components/CombatantComponent.h>
#include "UObject/ConstructorHelpers.h"
#include <GameFramework/HUD.h>
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "DroneRPG/Controllers/DroneRPGPlayerController.h"

ADroneRPGGameMode::ADroneRPGGameMode(): gameMode()
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
	UCombatantComponent* damageDealer = mGetCombatantComponent(damager);
	// Check if the damager uses the UDroneDamagerInterface, which it should
	if (IsValid(damageDealer)) 
	{
		// Get their team and their teams current score.
		float& score = teamScores.FindOrAdd(damageDealer->GetTeam());

		//  TODO The game mode may want to override something called GetKilledScore etc. so the amount of points can change per game mode
		// Add extra points.
		score++;
	}
}

void ADroneRPGGameMode::AddTeamScore(int32 team, float bonusScore)
{
	// Find and add the points to the team, from AObjective
	float& score = teamScores.FindOrAdd(team);
	score += bonusScore;
}

FString ADroneRPGGameMode::GetTeamScoreText(int32 team)
{
	// Create a basic team score text string using colour and points
	FColor tc = UFunctionLibrary::GetTeamColour(team);
	float& score = teamScores.FindOrAdd(team);

	TArray< FStringFormatArg > args;
	args.Add(FStringFormatArg(UFunctionLibrary::GetColourString(tc)));
	args.Add(FStringFormatArg(static_cast<int>(FMath::RoundHalfToEven(score))));
	return FString::Format(TEXT("Team {0} has {1} points"), args);
}