#include "PlasmaStormGameMode.h"
#include <Kismet/KismetSystemLibrary.h>
#include <Kismet/GameplayStatics.h>

#include "DroneRPG/DroneDamagerInterface.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"

void APlasmaStormGameMode::EntityKilled(AActor* killedEntity, AActor* damager)
{
	// Check how this works with the storm? Are we getting Team 0?
	Super::EntityKilled(killedEntity, damager);

	// Check if the damager was an APlasmaStorm, if so add kills
	IDroneDamagerInterface* damageDealer = Cast<IDroneDamagerInterface>(damager);
	if (damageDealer->GetDamagerType() == EDamagerType::PlasmaStorm) {
		kills++;
	}

	// End the game after 100 kills
	if (kills >= 100) {
		for (APlayerState* ps : UGameplayStatics::GetGameState(GetWorld())->PlayerArray) {
			APlayerController* con = UGameplayStatics::GetPlayerController(GetWorld(), ps->GetPlayerId());
			UKismetSystemLibrary::QuitGame(GetWorld(), con, EQuitPreference::Quit, false);
		}
	}
}

TArray<FScoreBoardStat> APlasmaStormGameMode::GetScoreBoardStats()
{
	TArray<FScoreBoardStat> stats;

	// Get the basic team scores
	for (auto& team : UFunctionLibrary::GetTeamColours()) {
		FScoreBoardStat stat;
		stat.text = GetTeamScoreText(team.Key);
		stat.textColour = team.Value;
		stats.Add(stat);
	}

	// Add plasma storm kills to the score board
	TArray< FStringFormatArg > args;
	args.Add(FStringFormatArg(static_cast<int>(FMath::RoundHalfToEven(kills))));

	stats.Add(FScoreBoardStat(FString::Format(TEXT("Plasma Storm has {0} kills"), args), FColor::Purple));
	return stats;
}