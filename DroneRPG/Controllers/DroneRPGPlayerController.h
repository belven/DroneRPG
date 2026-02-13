#pragma once
#include "CoreMinimal.h"
#include "DroneRPG/Utilities/Enums.h"
#include "GameFramework/PlayerController.h"
#include "DroneRPGPlayerController.generated.h"

class ADroneRPGGameMode;
class ADroneRPGCharacter;

UCLASS()
class ADroneRPGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADroneRPGPlayerController();

	UFUNCTION(BlueprintCallable, Category = "Drone Controller")
	ADroneRPGCharacter* GetDrone() const;
protected:
	/* The speed our ship moves around the level */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		float MoveSpeed;
	bool moveCamera;
	int32 droneIndex;

	UPROPERTY()
	ADroneRPGCharacter* droneCharacter;

	bool isFiring;

	void One();
	void Two();
	void Three();
	void Four();
	void Five();

	FTimerHandle TimerHandle_CameraTimer;

	void SetWeapon(EWeaponType type);
	// Static names for axis bindings
	static const FName MoveForwardBinding;
	static const FName MoveRightBinding;
	static const FName FireForwardBinding;
	static const FName FireRightBinding;

	virtual void PlayerTick(float DeltaTime) override;
	void FireShot(const FVector& FireDirection);
	void UseTool();
	void StopUsingTool();
	void CalculateMovement(float DeltaSeconds) const;
	virtual void SetupInputComponent() override;
	void CanMoveCamera();
	void IncrementDrone();
	ADroneRPGGameMode* GetGameMode();
	virtual void OnPossess(APawn* aPawn) override;

private:
	UFUNCTION()
	void ChangeView();

	UPROPERTY()
	ADroneRPGGameMode* gameMode;
};