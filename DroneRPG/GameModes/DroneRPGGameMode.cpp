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
		AddTeamScore(damageDealer->GetTeam(), 1);
	}
}

void ADroneRPGGameMode::AddTeamScore(int32 team, int32 bonusScore)
{
	FTeamScore* teamScore = teamScores.Find(team);
	if (teamScore != NULL) 
	{
		teamScore->score += bonusScore;
	}
	else
	{
		teamScores.Add(team, FTeamScore(team, UFunctionLibrary::GetTeamColour(team), bonusScore));
	}
}

FString ADroneRPGGameMode::GetTeamScoreText(int32 team)
{
	FTeamScore& teamScore = teamScores.FindOrAdd(team);
	return teamScore.GetTeamScoreText();
}