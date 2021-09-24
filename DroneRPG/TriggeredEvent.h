// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TriggeredEvent.generated.h"

UCLASS()
class DRONERPG_API ATriggeredEvent : public AActor
{
	GENERATED_BODY()
	
public:	

	ATriggeredEvent();

	virtual void TriggerEvent();

	virtual void Tick(float DeltaTime) override;
protected:
	FTimerHandle TimerHandle_EventTrigger;
	virtual void BeginPlay() override;
};
