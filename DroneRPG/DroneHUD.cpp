#include "DroneHUD.h"
#include "FunctionLibrary.h"
#include "DroneRPGCharacter.h"
#include <Kismet/GameplayStatics.h>
#include "DroneRPGPlayerController.h"
#include "Objective.h"

#define mGetController Cast<ADroneRPGPlayerController>(GetPlayerDrone()->GetController())
#define mProjectWorldToScreen(con, actor, pos) UGameplayStatics::ProjectWorldToScreen(con, actor->GetActorLocation(), pos)

TArray<ADroneRPGCharacter*> ADroneHUD::GetDrones()
{
	return mGetActorsInWorld<ADroneRPGCharacter>(GetWorld());
}

void ADroneHUD::DrawHUD() {
	Super::DrawHUD();

	for (ADroneRPGCharacter* drone : GetEnemyDrones()) {
		if (drone->IsAlive())
			DrawEnemyIndicators(drone);
	}

	for (AObjective* objective : mGetActorsInWorld<AObjective>(GetWorld()))
	{
		DrawObjectiveIndicators(objective);
	}
}

void ADroneHUD::DrawObjectiveIndicators(AObjective* objective) {
	ADroneRPGPlayerController* con = mGetController;

	int32 vpX;
	int32 vpY;

	FVector screenPos = Project(objective->GetActorLocation());

	float screenX = screenPos.X;
	float screenY = screenPos.Y;

	con->GetViewportSize(vpX, vpY);

	float clampY = mClampValue<float>(screenY, vpY - 50, 50);
	float clampX = mClampValue<float>(screenX, vpX - 50, 50);

	if (clampY != screenY || clampX != screenX) {
		float x = clampX;
		float y = clampY;

		float startX = x - 5;
		float startY = y;
		float endX = x + 5;
		float endY = y;

		DrawLine(startX, startY, endX, endY, FLinearColor(objective->GetCurrentColour()), 10.0f);
	}

}

void ADroneHUD::DrawEnemyIndicators(ADroneRPGCharacter* drone) {
	ADroneRPGPlayerController* con = mGetController;

	int32 vpX;
	int32 vpY;
	
	FVector screenPos = Project(drone->GetActorLocation());
	   
	float screenX = screenPos.X;
	float screenY = screenPos.Y;

	con->GetViewportSize(vpX, vpY);

	float clampY = mClampValue<float>(screenY, vpY - 50, 50);
	float clampX = mClampValue<float>(screenX, vpX - 50, 50);

	if (clampY == screenY && clampX == screenX) {
		float x = clampX;
		float y = clampY - 30;

		float startX = x - 10;
		float startY = y;
		float endX = x + 10;
		float endY = y;

		DrawLine(startX, startY, endX, endY, FLinearColor(drone->GetHealthStatus()), 5.0f);
	}
	/*TArray< FStringFormatArg > args;
	args.Add(FStringFormatArg(clampX));
	args.Add(FStringFormatArg(clampY));

	DrawText(FString::Format(TEXT("X = {0}, Y = {1}"), args), FLinearColor(FColor::Red), clampX, clampY);*/
}

float ADroneHUD::ClampValueMinus(float value, float max, float modifier) {
	value = mClampValue<float>(value, max - modifier, -max + modifier);
	return value;
}

TArray<ADroneRPGCharacter*> ADroneHUD::GetEnemyDrones() {
	TArray<ADroneRPGCharacter*> drones;

	if (GetPlayerDrone() != NULL) {
		for (ADroneRPGCharacter* drone : GetDrones()) {

			if (GetPlayerDrone()->GetTeam() != drone->GetTeam()) {
				drones.Add(drone);
			}
		}
	}

	return drones;
}