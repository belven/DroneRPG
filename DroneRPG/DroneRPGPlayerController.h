#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Weapon.h"
#include "DroneRPGPlayerController.generated.h"

UCLASS()
class ADroneRPGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADroneRPGPlayerController();

	UFUNCTION(BlueprintCallable, Category = "Drone Controller")
	class ADroneRPGCharacter* GetDrone();
protected:
	/* The speed our ship moves around the level */
	UPROPERTY(Category = Gameplay, EditAnywhere, BlueprintReadWrite)
		float MoveSpeed;
	bool moveCamera;
	int32 droneIndex;

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
	void FireShot(FVector FireDirection);
	void UseTool();
	void StopUsingTool();
	void CalculateMovement(float DeltaSeconds);
	virtual void SetupInputComponent() override;
	void CanMoveCamera();
	void IncrementDrone();
private:
	void ChangeView();
};