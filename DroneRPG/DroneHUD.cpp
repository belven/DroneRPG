#include "DroneHUD.h"
#include "FunctionLibrary.h"
#include "DroneRPGCharacter.h"
#include <Kismet/GameplayStatics.h>
#include "DroneRPGPlayerController.h"
#include "Objective.h"

#define mGetController Cast<ADroneRPGPlayerController>(GetPlayerDrone()->GetController())

void ADroneHUD::DrawHUD() {
	Super::DrawHUD();
	DrawScore();

	// Get all the enemy drones in the game and display indicators where appropriate
	for (ADroneRPGCharacter* drone : GetEnemyDrones()) {
		if (drone->IsAlive())
			DrawEnemyIndicators(drone);
	}

	// Get all the objectives in the game and display indicators where appropriate
	for (AObjective* objective : mGetActorsInWorld<AObjective>(GetWorld()))
	{
		DrawObjectiveIndicators(objective);
	}
}

void ADroneHUD::DrawScore() {
	ADroneRPGPlayerController* con = mGetController;
	TMap<int32, int32> totalScore;

	int32 y = 20;
	int32 vpX;
	int32 vpY;

	// Get the viewport (current window) size
	con->GetViewportSize(vpX, vpY);

	for (ADroneRPGCharacter* drone : mGetDrones) {
		totalScore.FindOrAdd(drone->GetTeam()) += drone->GetKills();
	}

	for (auto& pair : totalScore) {
		FColor tc = *UFunctionLibrary::GetTeamColours().Find(pair.Key);
		TArray< FStringFormatArg > args;
		args.Add(FStringFormatArg(UFunctionLibrary::GetColourString(tc)));
		args.Add(FStringFormatArg(pair.Value));

		// Write some text below the drone that states it's current kills and deaths
		DrawText(FString::Format(TEXT("{0} = {1}"), args), FLinearColor(tc), vpX / 2, y);
		y += 20;
	}
}

void ADroneHUD::DrawObjectiveIndicators(AObjective* objective) {
	ADroneRPGPlayerController* con = mGetController;

	if (con != NULL) {
		int32 vpX;
		int32 vpY;
		int32 offset = 30;

		// Get the objective screen location
		FVector screenPos = Project(objective->GetActorLocation());

		float screenX = screenPos.X;
		float screenY = screenPos.Y;

		// Get the viewport (current window) size
		con->GetViewportSize(vpX, vpY);

		// Limit the objective location to the screen
		float clampY = mClampValue<float>(screenY, vpY - offset, offset);
		float clampX = mClampValue<float>(screenX, vpX - offset, offset);

		float x = clampX;
		float y = clampY;

		float startX = x - 5;
		float startY = y;
		float endX = x + 5;
		float endY = y;

		// Only display a indicator if the objective is off screen
		// If we've had to clamp a value, then the position is off screen
		if (clampY != screenY || clampX != screenX) {
			// Draw a Line on the edge of the screen that's the colour of the objective
			DrawLine(startX, startY, endX, endY, FLinearColor(objective->GetCurrentColour()), 15.0f);
		}
		else {
			TArray< FStringFormatArg > args;
			FString control = FString::FormatAsNumber((int32)FMath::RoundHalfToEven(objective->GetCurrentControlPercent()));
			args.Add(FStringFormatArg(control));

			// Write some text below the drone that states it's current kills and deaths
			DrawText(FString::Format(TEXT("{0}%"), args), FLinearColor(objective->GetCurrentColour()), clampX, clampY + 30);
		}
	}
}

void ADroneHUD::DrawEnemyIndicators(ADroneRPGCharacter* drone) {
	ADroneRPGPlayerController* con = mGetController;

	if (con != NULL) {
		int32 vpX;
		int32 vpY;
		int32 offset = 30;

		// Get the drone screen location
		FVector screenPos = Project(drone->GetActorLocation());

		float screenX = screenPos.X;
		float screenY = screenPos.Y;

		// Get the viewport (current window) size
		con->GetViewportSize(vpX, vpY);

		// Limit the drone location to the screen
		float clampY = mClampValue<float>(screenY, vpY - offset, offset);
		float clampX = mClampValue<float>(screenX, vpX - offset, offset);

		// Only display a indicator if the drone is on screen
		// If we've had to clamp a value, then the position is off screen
		if (clampY == screenY && clampX == screenX) {
			float x = clampX;
			float y = clampY - 30;

			float startX = x - 10;
			float startY = y;
			float endX = x + 10;
			float endY = y;

			// Draw a Line above the drone that has it's team colour
			//DrawLine(startX, startY, endX, endY, FLinearColor(drone->GetTeamColour()), 5.0f);

			TArray< FStringFormatArg > args;
			args.Add(FStringFormatArg(drone->GetKills()));
			args.Add(FStringFormatArg(drone->GetDeaths()));

			// Write some text below the drone that states it's current kills and deaths
			DrawText(FString::Format(TEXT("{0}/{1}"), args), FLinearColor(FColor::Red), clampX, clampY + 30);
		}
	}
}

TArray<ADroneRPGCharacter*> ADroneHUD::GetEnemyDrones() {
	return mGetEnemysInRadius(0, FVector::ZeroVector, GetPlayerDrone()->GetTeam());
}