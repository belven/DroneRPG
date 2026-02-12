#include "PlasmaStormGameMode.h"
#include <DroneRPG/Components/CombatantComponent.h>
#include <Kismet/KismetSystemLibrary.h>
#include <Kismet/GameplayStatics.h>
#include "DroneRPG/LevelActors/PlasmaStormEvent.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"

APlasmaStormGameMode::APlasmaStormGameMode() : kills(0)
{
	gameMode = EGameModeType::PlasmaStorm;
}

void APlasmaStormGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlasmaStormEvent* plasmaStorm = GetWorld()->SpawnActor<APlasmaStormEvent>(APlasmaStormEvent::StaticClass(), FVector(0, 0, 0), FRotator());
	FTeamScore();
	int team = -1;
	GetTeamScores().Add(team, FTeamScore(team,UFunctionLibrary::GetTeamColour(team), 0, "Plasma Storm"));
}

void APlasmaStormGameMode::EntityKilled(AActor* killedEntity, AActor* damager)
{
	// Check how this works with the storm? Are we getting Team 0?
	Super::EntityKilled(killedEntity, damager);

	// Check if the damager was an APlasmaStorm, if so add kills

	UCombatantComponent* combatantComponent = mGetCombatantComponent(damager);
	if (combatantComponent->GetCombatantType() == EDamagerType::PlasmaStorm) 
	{
		kills++;
	}

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