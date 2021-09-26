#include "PlasmaStormGameMode.h"
#include "DroneDamagerInterface.h"
#include "FunctionLibrary.h"
#include <Kismet/KismetSystemLibrary.h>
#include <Kismet/GameplayStatics.h>
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"

void APlasmaStormGameMode::EntityKilled(AActor* killedEntity, AActor* damager)
{
	Super::EntityKilled(killedEntity, damager);

	IDroneDamagerInterface* damageDealer = Cast<IDroneDamagerInterface>(damager);
	if (damageDealer->GetDamagerType() == EDamagerType::PlasmaStorm) {
		kills++;
	}

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

	for (auto& team : UFunctionLibrary::GetTeamColours()) {
		FScoreBoardStat stat;
		stat.text = GetTeamScoreText(team.Key);
		stat.textColour = team.Value;
		stats.Add(stat);
	}

	TArray< FStringFormatArg > args;
	args.Add(FStringFormatArg((int)FMath::RoundHalfToEven(kills)));

	stats.Add(FScoreBoardStat(FString::Format(TEXT("Plasma Storm has {0} kills"), args), FColor::Purple));
	return stats;
}