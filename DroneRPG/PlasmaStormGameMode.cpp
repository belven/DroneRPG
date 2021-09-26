#include "PlasmaStormGameMode.h"
#include "DroneDamagerInterface.h"
#include "FunctionLibrary.h"

void APlasmaStormGameMode::EntityKilled(AActor* killedEntity, AActor* damager)
{
	IDroneDamagerInterface* damageDealer = Cast<IDroneDamagerInterface>(damager);
	if (damageDealer->GetDamagerType() == EDamagerType::PlasmaStorm) {
		kills++;
	}
}