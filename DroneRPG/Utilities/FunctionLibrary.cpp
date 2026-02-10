#include "FunctionLibrary.h"
#include "DroneRPG/DroneRPGCharacter.h"

TMap<int32, FColor> UFunctionLibrary::teamColours = GetTeamColours();

TArray <ADroneRPGCharacter*> UFunctionLibrary::GetEnemiesInRadius(float radius, const FVector& loc, int32 team, TArray <ADroneRPGCharacter*>& allDrones)
{
	TArray<ADroneRPGCharacter*> drones;

	for (ADroneRPGCharacter* drone : mGetDronesInRadius(radius, loc, allDrones))
	{
		// Check if the drone found is an enemy
		if (team != drone->GetTeam()) {
			drones.Add(drone);
		}
	}

	return drones;
}

TArray <ADroneRPGCharacter*> UFunctionLibrary::GetDronesInRadius(float radius, const FVector& loc, TArray <ADroneRPGCharacter*>& drones)
{
	TArray<ADroneRPGCharacter*> dronesInRange;

	for (ADroneRPGCharacter* drone : drones)
	{
		float dist = mDist(drone->GetActorLocation(), loc);

		if (radius == 0 || dist <= radius) {
			dronesInRange.Add(drone);
		}
	}

	return dronesInRange;
}


ADroneRPGCharacter* UFunctionLibrary::GetClosestEnemyInRadius(float radius, const FVector& loc, int32 team, TArray <ADroneRPGCharacter*>& allDrones)
{
	return mGetClosestActorInArray<ADroneRPGCharacter>(GetEnemiesInRadius(radius, loc, team, allDrones), loc);
}

FString UFunctionLibrary::GetColourString(FColor color)
{
	FString colourString = "";

	if (color == FColor::Red) {
		colourString = "Red";
	}
	else if (color == FColor::Green) {
		colourString = "Green";
	}
	else if (color == FColor::Yellow) {
		colourString = "Yellow";
	}
	else if (color == FColor::Silver) {
		colourString = "Silver";
	}
	else if (color == FColor::Black) {
		colourString = "Black";
	}
	else if (color == FColor::Blue) {
		colourString = "Blue";
	}
	else if (color == FColor::Magenta) {
		colourString = "Magenta";
	}
	else if (color == FColor::Cyan) {
		colourString = "Cyan";
	}
	else if (color == FColor::Emerald) {
		colourString = "Emerald";
	}
	else if (color == FColor::Purple) {
		colourString = "Purple";
	}
	else if (color == FColor::Orange) {
		colourString = "Orange";
	}
	else if (color == FColor::Turquoise) {
		colourString = "Turquoise";
	}
	else if (color == FColor::White) {
		colourString = "White";
	}

	return colourString;
}


FColor UFunctionLibrary::GetTeamColour(int32 team)
{
	FColor colour = FColor::Blue;

	if (GetTeamColours().Contains(team))
	{
		colour = *GetTeamColours().Find(team);
	}
	return colour;
}

TMap<int32, FColor>& UFunctionLibrary::GetTeamColours()
{
	if (teamColours.IsEmpty()) {
		teamColours.Add(0, FColor::Green);
		teamColours.Add(1, FColor::Blue);
		teamColours.Add(2, FColor::Yellow);
		teamColours.Add(3, FColor::White);
		teamColours.Add(4, FColor::Magenta);
	}
	return teamColours;
}