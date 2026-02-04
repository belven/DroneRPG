#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KeyActor.generated.h"

UCLASS()
class DRONERPG_API AKeyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AKeyActor();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KeyActor")
	float keyActorSize;

	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	float GetSize() const { return keyActorSize; }
	void SetSize(float val) { keyActorSize = val; }
};
