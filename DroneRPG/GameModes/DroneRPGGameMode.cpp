#include "DroneRPGGameMode.h"
#include <DroneRPG/Components/CombatantComponent.h>
#include "UObject/ConstructorHelpers.h"
#include <GameFramework/HUD.h>
#include "DroneRPG/Controllers/DroneRPGPlayerController.h"

ADroneRPGGameMode::ADroneRPGGameMode() : gameMode(), coloursSet(false)
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

	colours.Add(FColor::Red);
	colours.Add(FColor::Black);
}

void ADroneRPGGameMode::UnitHit(float inDamage, UCombatantComponent* attacker)
{
		float bonusScore = FMath::RoundHalfToEven(inDamage);
		attacker->AddCombatScore(bonusScore);
		AddTeamScore(attacker->GetTeam(), bonusScore);	
}

void ADroneRPGGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void ADroneRPGGameMode::EntityKilled(UCombatantComponent* killedEntity, UCombatantComponent* killer)
{

		AddTeamScore(killer->GetTeam(), 50);
		killer->AddCombatScore(50);

		TArray< FStringFormatArg > args;
		args.Add(FStringFormatArg(killedEntity->GetCombatantName()));
		args.Add(FStringFormatArg(killer->GetCombatantName()));

		// Add text to the kill feed
		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::White, FString::Format(TEXT("{0} was killed by {1}"), args));
	
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
		teamScores.Add(team, FTeamScore(team, GetTeamColour(team), bonusScore));
	}
}

FString ADroneRPGGameMode::GetTeamScoreText(int32 team)
{
	FTeamScore& teamScore = teamScores.FindOrAdd(team);
	return teamScore.GetTeamScoreText();
}

FColor ADroneRPGGameMode::GetTeamColour(int32 team)
{
	FColor colour = FColor::Blue;

	if (GetTeamColours().Contains(team))
	{
		colour = *GetTeamColours().Find(team);
	}
	else
	{
		colour = FColor::MakeRandomColor();

		while (colours.Contains(colour)) 
		{
			colour = FColor::MakeRandomColor();
		}

		colours.Add(colour);
		teamColours.Add(team, colour);

		//if (colours.IsEmpty() && !coloursSet)
		//{
		//	coloursSet = true;
		//	colours.Add(FColor::White);
		//	colours.Add(FColor::Red);
		//	colours.Add(FColor::Green);
		//	colours.Add(FColor::Blue);
		//	colours.Add(FColor::Yellow);
		//	colours.Add(FColor::Cyan);
		//	colours.Add(FColor::Magenta);
		//	colours.Add(FColor::Orange);
		//	colours.Add(FColor::Purple);
		//	colours.Add(FColor::Turquoise);
		//	colours.Add(FColor::Emerald);

		//	UFunctionLibrary::ShuffleArray(colours);
		//}
		//else if (colours.IsEmpty() && coloursSet)
		//{
		//	colour = FColor::MakeRandomColor();
		//	teamColours.Add(team, colour);
		//}
		//else
		//{
		//	colour = colours.Pop();
		//	teamColours.Add(team, colour);
		//}
	}
	return colour;
}

TMap<int32, FColor>& ADroneRPGGameMode::GetTeamColours()
{
	return teamColours;
}