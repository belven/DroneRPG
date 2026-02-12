#include "DroneHUD.h"
#include <Kismet/GameplayStatics.h>
#include "DroneRPG/DroneRPGCharacter.h"
#include "DroneRPG/Components/HealthComponent.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "DroneRPG/GameModes/DroneRPGGameMode.h"
#include "DroneRPG/LevelActors/Objective.h"

UCombatantComponent* ADroneHUD::GetCombatantComponent()
{
	return combatantComponent;
}

FDrawLocation ADroneHUD::GetDrawLocation(const FVector& worldLocation, double offset)
{
	FDrawLocation drawLocation = FDrawLocation();
	FVector2D viewportSize = GetViewportSize();

	FVector screenPos = Project(worldLocation);

	// Get the viewport (current window) size

	// Limit the objective location to the screen
	drawLocation.Y = mClampValue(screenPos.Y, viewportSize.Y - offset, offset);
	drawLocation.X = mClampValue(screenPos.X, viewportSize.X - offset, offset);

	if (drawLocation.Y != screenPos.Y)
	{
		drawLocation.yOffscreen = true;
	}

	if (drawLocation.X != screenPos.X)
	{
		drawLocation.xOffscreen = true;
	}
	return drawLocation;
}

FVector2D ADroneHUD::GetViewportSize()
{
	int32 x;
	int32 y;
	GetOwningPlayerController()->GetViewportSize(x, y);
	return FVector2D(x,y);
}

void ADroneHUD::DrawHUD()
{
	Super::DrawHUD();
	DrawScore();

	// Get all the enemy drones in the game and display indicators where appropriate
	for (ADroneRPGCharacter* drone : GetGameMode()->GetDrones())
	{
		if (drone->GetHealthComponent()->IsAlive())
		{
			DrawCombatantIndicators(drone);
		}
	}

	// Get all the objectives in the game and display indicators where appropriate
	for (AObjective* objective : mGetActorsInWorld<AObjective>(GetWorld()))
	{
		DrawObjectiveIndicators(objective);
	}
}

void ADroneHUD::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(GetOwningPlayerController()) && IsValid(GetOwningPlayerController()->GetPawn()))
	{
		SetCombatantComponent(mGetCombatantComponent(GetOwningPlayerController()->GetPawn()));
	}
}

void ADroneHUD::DrawScore()
{
	int32 y = 20;
	FVector2D viewportSize = GetViewportSize();

	for (FScoreBoardStat stat : GetGameMode()->GetScoreBoardStats())
	{
		DrawText(stat.text, FLinearColor(stat.textColour), viewportSize.X / 2.1, y);
		y += 20;
	}
}

void ADroneHUD::DrawObjectiveIndicators(AObjective* objective)
{
	if (GetOwningPlayerController() != NULL)
	{
		constexpr int32 offset = 30;
		FDrawLocation drawLocation = GetDrawLocation(objective->GetActorLocation(), offset);

		// Only display an indicator if the objective is offscreen
		// If we've had to clamp a value, then the position is offscreen
		if (drawLocation.IsOffscreen())
		{
			FVector2D start = FVector2D(drawLocation.X - 5, drawLocation.Y);
			FVector2D end = FVector2D(drawLocation.X + 5, drawLocation.Y);

			// Draw a Line on the edge of the screen that's the colour of the objective
			DrawLine(start.X, start.Y, end.X, end.Y, FLinearColor(objective->GetCurrentColour()), 15.0f);
		}
		else
		{
			TArray< FStringFormatArg > args;
			FString control = FString::SanitizeFloat(FMath::RoundHalfToEven(objective->GetCurrentControlPercent()));
			args.Add(FStringFormatArg(control));

			// Write some text below the drone that states it's current kills and deaths
			DrawText(FString::Format(TEXT("{0}%"), args), FLinearColor(objective->GetCurrentColour()), drawLocation.X, drawLocation.Y + 30);
		}
	}
}

void ADroneHUD::DrawCombatantIndicators(ADroneRPGCharacter* drone)
{
	if (GetOwningPlayerController() != NULL)
	{
		constexpr int32 offset = 30;
		FDrawLocation drawLocation = GetDrawLocation(drone->GetActorLocation(), offset);

		// Only display a indicator if the drone is on screen
		// If we've had to clamp a value, then the position is offscreen
		if (!drawLocation.IsOffscreen())
		{
			TArray<FStringFormatArg> args;
			args.Add(FStringFormatArg(drone->GetKills()));
			args.Add(FStringFormatArg(drone->GetDeaths()));

			FColor colour = FColor::Red;

			if (drone->GetTeam() == GetCombatantComponent()->GetTeam())
			{
				colour = FColor::Green;
			}

			// Write some text below the drone that states it's current kills and deaths
			DrawText(FString::Format(TEXT("{0}/{1}"), args), FLinearColor(colour), drawLocation.X, drawLocation.Y + 30);
		}
	}
}

ADroneRPGGameMode* ADroneHUD::GetGameMode()
{
	if (!IsValid(gameMode))
	{
		gameMode = Cast<ADroneRPGGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	}
	return gameMode;
}