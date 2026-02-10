#pragma once
#include "CoreMinimal.h"
#include <EngineUtils.h>
#include <Engine/World.h>
#include "FunctionLibrary.generated.h"

class ADroneRPGCharacter;
class UWeapon;

#define MIN(a,b) (a < b) ? (a) : (b)
#define MAX(a,b) (a > b) ? (a) : (b)

#define mDist(a, b) FVector::Dist(a, b)
#define  mIsA(aObject, aClass)  aObject->IsA(aClass::StaticClass())
#define  mImplements(aObject, aClass)  aObject->Implements<aClass>()
#define  mAddOnScreenDebugMessage(text) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT(text)));
#define mDroneLocation GetCharacter()->GetActorLocation()
#define mDroneRotation GetCharacter()->GetActorRotation()

#define mClampValue UFunctionLibrary::ClampValue
#define mShuffleArray UFunctionLibrary::ShuffleArray
#define mGetRandomObject UFunctionLibrary::GetRandomObject

#define mGetActorsInWorld UFunctionLibrary::GetActorsInWorld
#define mGetClosestActorInWorld UFunctionLibrary::GetClosestActorInWorld
#define mGetClosestActorInArray UFunctionLibrary::GetClosestActorInArray
#define mGetActorsInRadius UFunctionLibrary::GetActorsInRadius
#define mGetDronesInRadius UFunctionLibrary::GetDronesInRadius

#define mGetEnemiesInRadius UFunctionLibrary::GetEnemiesInRadius
#define mGetClosestEnemyInRadius UFunctionLibrary::GetClosestEnemyInRadius

#define mRandomReachablePointInRadius(start, radius, loc) UNavigationSystemV1::GetCurrent(GetWorld())->GetRandomReachablePointInRadius(start, radius, loc);
#define mRandomPointInNavigableRadius(start, radius, loc) UNavigationSystemV1::GetCurrent(GetWorld())->GetRandomPointInNavigableRadius(start, radius, loc);
#define mSetTimer(handle, method, delay) GetWorld()->GetTimerManager().SetTimer(handle, this, method, delay)
#define mSetTimerWorld(world, handle, method, delay) world->GetTimerManager().SetTimer(handle, this, method, delay)

UCLASS()
class DRONERPG_API UFunctionLibrary : public UObject
{
	GENERATED_BODY()
public:
	template <class T> static TArray<T*> GetActorsInWorld(UWorld* world);
	template <class T> static TArray <T*> GetActorsInRadius(UWorld* world, float radius, FVector loc);

	template <class T> static T* GetClosestActorInWorld(UWorld* world, FVector loc);
	template <class T> static T* GetClosestActorInArray(TArray<T*> actorArray, FVector loc);

	static TArray <ADroneRPGCharacter*> GetEnemiesInRadius(float radius, const FVector& loc, int32 team, TArray <ADroneRPGCharacter*>& allDrones);
	static ADroneRPGCharacter* GetClosestEnemyInRadius(float radius, const FVector& loc, int32 team, TArray <ADroneRPGCharacter*>& allDrones);

	static TArray <ADroneRPGCharacter*> GetDronesInRadius(float radius, const FVector& loc, TArray <ADroneRPGCharacter*>& drones);

	template<class T> static	T ClampValue(T value, T max, T min);
	template <class T> static void ShuffleArray(TArray<T>& arrayIn);
	template <class T> static T GetRandomObject(TArray<T>& arrayIn);

	static FColor GetTeamColour(int32 team);
	static  TMap<int32, FColor>& GetTeamColours();

	static FString GetColourString(FColor color);
	template<class T> static T GetRandomEnum(T end);

private:
	static TMap<int32, FColor> teamColours;
};

template <class T>
TArray <T*> UFunctionLibrary::GetActorsInRadius(UWorld* world, float radius, FVector loc) {
	TArray<T*> actors = mGetActorsInWorld<T>(world);
	TArray<T*> actorsInRange;

	for (T* actor : actors)
	{
		float dist = mDist(actor->GetActorLocation(), loc);

		if (dist <= radius) {
			actorsInRange.Add(actor);
		}
	}

	return actorsInRange;
}

template <class T>
T* UFunctionLibrary::GetClosestActorInArray(TArray<T*> actorArray, FVector loc) {
	T* closest = NULL;
	float lastDist = 0;

	for (T* actor : actorArray)
	{
		float dist = mDist(actor->GetActorLocation(), loc);

		if (closest == NULL) {
			closest = actor;
			lastDist = dist;
		}
		else if (dist < lastDist) {
			closest = actor;
			lastDist = dist;
		}
	}
	return closest;
}

template <class T>
T* UFunctionLibrary::GetClosestActorInWorld(UWorld* world, FVector loc) {
	return mGetClosestActorInArray<T>(mGetActorsInWorld<T>(world), loc);
}

template <class T>
TArray<T*> UFunctionLibrary::GetActorsInWorld(UWorld* world) {
	TArray<T*> actors;

	for (TActorIterator<T> actorItr(world); actorItr; ++actorItr)
	{
		actors.Add(*actorItr);
	}

	return actors;
}

template<class T>
T UFunctionLibrary::ClampValue(T value, T max, T min)
{
	if (value < min)
		value = min;

	if (value > max)
		value = max;

	return value;
}

template <class T>
void UFunctionLibrary::ShuffleArray(TArray<T>& arrayIn)
{
	if (arrayIn.Num() > 0)
	{
		int32 LastIndex = arrayIn.Num() - 1;
		for (int32 i = 0; i <= LastIndex; ++i)
		{
			int32 Index = FMath::RandRange(i, LastIndex);
			if (i != Index)
			{
				arrayIn.Swap(i, Index);
			}
		}
	}
}

template <class T>
T UFunctionLibrary::GetRandomObject(TArray<T>& arrayIn)
{
	return arrayIn[FMath::RandRange(0, arrayIn.Num() - 1)];
}

template<class T>
T UFunctionLibrary::GetRandomEnum(T end)
{
	return static_cast<T>(rand() % (static_cast<int>(end) - 1));
}