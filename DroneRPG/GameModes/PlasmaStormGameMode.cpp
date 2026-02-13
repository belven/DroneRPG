#include "PlasmaStormGameMode.h"
#include <DroneRPG/Components/CombatantComponent.h>
#include <Kismet/KismetSystemLibrary.h>
#include <Kismet/GameplayStatics.h>
#include "DroneRPG/LevelActors/PlasmaStormEvent.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"

APlasmaStormGameMode::APlasmaStormGameMode() : kills(0)
{
	gameMode = EGameModeType::PlasmaStorm;
}

void APlasmaStormGameMode::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->SpawnActor<APlasmaStormEvent>(APlasmaStormEvent::StaticClass(), FVector(0, 0, 0), FRotator());
	int team = -1;
	GetTeamScores().Add(team, FTeamScore(team, GetTeamColour(team), 0, "Plasma Storm"));
}

void APlasmaStormGameMode::EntityKilled(UCombatantComponent* killedEntity, UCombatantComponent* damager)
{
	// Check how this works with the storm? Are we getting Team 0?
	Super::EntityKilled(killedEntity, damager);

	// Check if the damager was an APlasmaStorm, if so add kills
	if (damager->GetCombatantType() == EDamagerType::PlasmaStorm)
	{
		kills++;

		// End the game after 100 kills
		if (kills >= 100)
		{
			for (APlayerState* ps : UGameplayStatics::GetGameState(GetWorld())->PlayerArray)
			{
				APlayerController* con = UGameplayStatics::GetPlayerController(GetWorld(), ps->GetPlayerId());
				UKismetSystemLibrary::QuitGame(GetWorld(), con, EQuitPreference::Quit, false);
			}
		}
	}
}