// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include <EngineUtils.h>
#include <Engine/World.h>
#include "Weapon.h"
#include "FunctionLibrary.generated.h"

#define mDist(a, b) FVector::Dist(a, b)
#define  mIsA(aObject, aClass)  aObject->IsA(aClass::StaticClass())
#define  mAddOnScreenDebugMessage(text) GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT(text)));
#define mDroneLocation GetCharacter()->GetActorLocation()
#define mDroneRotation GetCharacter()->GetActorRotation()

#define mClampValue UFunctionLibrary::ClampValue
#define mShuffleArray UFunctionLibrary::ShuffleArray
#define mGetActorsInWorld UFunctionLibrary::GetActorsInWorld

#define mRandomReachablePointInRadius(start, radius, loc) UNavigationSystemV1::GetCurrent(GetWorld())->GetRandomReachablePointInRadius(start, radius, loc);
#define mSetTimer(handle, method, delay) GetWorld()->GetTimerManager().SetTimer(handle, this, method, delay)
#define mSetTimerWolrd(world, handle, method, delay) world->GetTimerManager().SetTimer(handle, this, method, delay)


UCLASS()
class DRONERPG_API UFunctionLibrary : public UObject
{
	GENERATED_BODY()
public:
	template <class T> static TArray<T*> GetActorsInWorld(UWorld* world);
	template<class T> static	T ClampValue(T value, T max, T min);
	template <class T> static void ShuffleArray(TArray<T>& arrayIn);
	template <class T> static T GetRandomObject(TArray<T>& arrayIn);
	static TMap<int32, FColor> GetTeamColours();
	static TMap<int32, FColor> teamColours;
	static UWeapon* GetWeapon(EWeaponType type, float inFireRate, float inDamage, ADroneRPGCharacter* inOwner);
	template<class T> static T GetRandomEnum(T end);
};

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
	return static_cast<T>(rand() % ((int)end - 1));
}

