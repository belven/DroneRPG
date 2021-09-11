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
	/* The speed our ship moves around the level */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		float MoveSpeed;

	bool isFiring;

	// Static names for axis bindings
	static const FName MoveForwardBinding;
	static const FName MoveRightBinding;
	static const FName FireForwardBinding;
	static const FName FireRightBinding;

	virtual void PlayerTick(float DeltaTime) override;
	void FireShot(FVector FireDirection);
	void UseTool();
	void StopUsingTool();
	void CalculateMovement(float DeltaSeconds);
	virtual void SetupInputComponent() override;
};