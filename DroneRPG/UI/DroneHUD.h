#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DroneHUD.generated.h"

class ADroneRPGGameMode;
class AObjective;
class ADroneRPGCharacter;

UCLASS()
class DRONERPG_API ADroneHUD : public AHUD
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "HUD")
		TArray<ADroneRPGCharacter*> GetEnemyDrones();

	UFUNCTION(BlueprintCallable, Category = "HUD")
		ADroneRPGCharacter* GetPlayerDrone() const { return playerDrone; }

	UFUNCTION(BlueprintCallable, Category = "HUD")
		void SetPlayerDrone(ADroneRPGCharacter* val) { playerDrone = val; }

	UFUNCTION(BlueprintCallable, Category = "HUD")
		virtual void DrawHUD() override;

	UFUNCTION(BlueprintCallable, Category = "HUD")
		void DrawScore();

	UFUNCTION(BlueprintCallable, Category = "HUD")
		void DrawObjectiveIndicators(AObjective* objective);

	UFUNCTION(BlueprintCallable, Category = "HUD")
		void DrawEnemyIndicators(ADroneRPGCharacter* drone);

		ADroneRPGGameMode* GetGameMode();
protected:
	UPROPERTY()
	ADroneRPGCharacter* playerDrone;

	UPROPERTY()
	ADroneRPGGameMode* gameMode;
};