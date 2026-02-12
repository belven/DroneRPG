#include "FunctionLibrary.h"

TMap<int32, FColor> UFunctionLibrary::teamColours = GetTeamColours();
TArray<FColor> UFunctionLibrary::colours;
bool UFunctionLibrary::coloursSet = false;

FString UFunctionLibrary::GetColourString(FColor color)
{
	return color.ToString();
}

FColor UFunctionLibrary::GetTeamColour(int32 team)
{
	FColor colour = FColor::Blue;

	if (GetTeamColours().Contains(team))
	{
		colour = *GetTeamColours().Find(team);
	}
	else
	{
		if (colours.IsEmpty() && !coloursSet)
		{
			coloursSet = true;
			colours.Add(FColor::White);
			colours.Add(FColor::Black);
			colours.Add(FColor::Transparent);
			colours.Add(FColor::Red);
			colours.Add(FColor::Green);
			colours.Add(FColor::Blue);
			colours.Add(FColor::Yellow);
			colours.Add(FColor::Cyan);
			colours.Add(FColor::Magenta);
			colours.Add(FColor::Orange);
			colours.Add(FColor::Purple);
			colours.Add(FColor::Turquoise);
			colours.Add(FColor::Silver);
			colours.Add(FColor::Emerald);

			ShuffleArray(colours);
		}

		colour = colours.Pop();
		teamColours.Add(team, colour);
	}
	return colour;
}

TMap<int32, FColor>& UFunctionLibrary::GetTeamColours()
{
	return teamColours;
}
