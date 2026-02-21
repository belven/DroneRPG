#pragma once
#include "CoreMinimal.h"
#include "BaseAIController.h"
#include "TurretController.generated.h"

class ATurret;

UCLASS()
class DRONERPG_API ATurretController : public ABaseAIController
{
	GENERATED_BODY()
public:
	virtual void SetFiringState(bool firingState) override;
	virtual void Tick(float DeltaSeconds) override;
	FVector GetPredictedLocation(AActor* actor);
	void RotateToFace(float DeltaSeconds);

	ATurret* GetTurret();

	void SetTurret(ATurret* inTurret) { turret = inTurret; }

private:
	UPROPERTY()
	ATurret* turret;
};