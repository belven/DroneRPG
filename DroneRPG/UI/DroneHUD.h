#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DroneHUD.generated.h"

class UCombatantComponent;
class ADroneRPGGameMode;
class AObjective;
class ADroneRPGCharacter;

USTRUCT(BlueprintType)
struct FDrawLocation
{
	GENERATED_USTRUCT_BODY()

	double X;
	double Y;

	bool xOffscreen = false;
	bool yOffscreen = false;

	FDrawLocation(): X(0), Y(0)	 {}

	bool IsOffscreen()
	{
		return xOffscreen || yOffscreen;
	}
};

UCLASS()
class DRONERPG_API ADroneHUD : public AHUD
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "HUD")
	UCombatantComponent* GetCombatantComponent();
	virtual void PostInitializeComponents() override;

	FDrawLocation GetDrawLocation(const FVector& worldLocation, double offset);

	UFUNCTION(BlueprintCallable, Category = "HUD")
		void SetCombatantComponent(UCombatantComponent* val) { combatantComponent = val; }

	UFUNCTION(BlueprintCallable, Category = "HUD")
		virtual void DrawHUD() override;

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "HUD")
		void DrawScore();

	UFUNCTION(BlueprintCallable, Category = "HUD")
		void DrawObjectiveIndicators(AObjective* objective);

	UFUNCTION(BlueprintCallable, Category = "HUD")
		void DrawCombatantIndicators(ADroneRPGCharacter* drone);

		ADroneRPGGameMode* GetGameMode();
protected:
	UPROPERTY()
	UCombatantComponent* combatantComponent;

	UPROPERTY()
	ADroneRPGGameMode* gameMode;
};