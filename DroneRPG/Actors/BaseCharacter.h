#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

class ADroneRPGGameMode;
class UCombatantComponent;
class UHealthComponent;
class UWeapon;

UCLASS()
class DRONERPG_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	UFUNCTION()
	virtual void UnitDied(UCombatantComponent* inKiller);
	// Sets default values for this character's properties
	ABaseCharacter();

	virtual void BeginPlay() override;
	FString GetCharacterName();

	virtual void PossessedBy(AController* NewController) override;

	UWeapon* GetWeapon() const { return weapon; }
	void SetWeapon(UWeapon* val) { weapon = val; }

		UFUNCTION(BlueprintCallable, Category = "Drone")
	int32 GetTeam() const;
	virtual void SetTeam(int32 val);

	UHealthComponent* GetHealthComponent() const { return healthComponent; }
	UCombatantComponent* GetCombatantComponent() const { return combatantComponent; }

	ADroneRPGGameMode* GetGameMode();

	UFUNCTION(BlueprintCallable, Category = "Drone")
	virtual void UnitHit(float damage, UCombatantComponent* attacker);

protected:
		UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* meshComponent;
private:
	UPROPERTY()
	UWeapon* weapon;
	FString characterName;

	UPROPERTY()
	UHealthComponent* healthComponent;

	UPROPERTY()
	UCombatantComponent* combatantComponent;

	UPROPERTY()
	ADroneRPGGameMode* gameMode;
};