// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DroneRPGPlayerController.generated.h"

UCLASS()
class ADroneRPGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADroneRPGPlayerController();

	class ADroneRPGCharacter* GetDrone();
protected:
	virtual void PlayerTick(float DeltaTime) override;
	void UseTool();
	virtual void SetupInputComponent() override;
};