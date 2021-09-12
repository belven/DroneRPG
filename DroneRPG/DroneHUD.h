// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DroneHUD.generated.h"

class ADroneRPGCharacter;

/**
 *
 */
UCLASS()
class DRONERPG_API ADroneHUD : public AHUD
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "HUD")
		TArray<ADroneRPGCharacter*> GetDrones();

	UFUNCTION(BlueprintCallable, Category = "HUD")
		TArray<ADroneRPGCharacter*> GetEnemyDrones();

	UFUNCTION(BlueprintCallable, Category = "HUD")
		ADroneRPGCharacter* GetPlayerDrone() const { return playerDrone; }

	UFUNCTION(BlueprintCallable, Category = "HUD")
		void SetPlayerDrone(ADroneRPGCharacter* val) { playerDrone = val; }

	UFUNCTION(BlueprintCallable, Category = "HUD")
		virtual void DrawHUD() override;

	UFUNCTION(BlueprintCallable, Category = "HUD")
		void DrawObjectiveIndicators(AObjective* objective);
	void DrawEnemyIndicators(ADroneRPGCharacter* drone);

	float ClampValueMinus(float value, float max, float modifier = 50.0f);
protected:
	ADroneRPGCharacter* playerDrone;

};
