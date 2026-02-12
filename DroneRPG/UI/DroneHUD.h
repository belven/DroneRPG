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

	bool IsOffscreen() { return xOffscreen || yOffscreen; }

	FDrawLocation() : X(0), Y(0) {}
};

UCLASS()
class DRONERPG_API ADroneHUD : public AHUD
{
	GENERATED_BODY()
public:
	FDrawLocation GetDrawLocation(const FVector& worldLocation, double offset);
	FVector2D GetViewportSize();

	virtual void DrawHUD() override;
	void DrawScore();
	void DrawObjectiveIndicators(AObjective* objective);
	void DrawCombatantIndicators(ADroneRPGCharacter* drone);

	ADroneRPGGameMode* GetGameMode();
	UCombatantComponent* GetCombatantComponent();
	void SetCombatantComponent(UCombatantComponent* val) { combatantComponent = val; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	UCombatantComponent* combatantComponent;

	UPROPERTY()
	ADroneRPGGameMode* gameMode;
};