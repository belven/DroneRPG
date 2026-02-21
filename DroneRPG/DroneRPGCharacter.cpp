#pragma once
#include "DroneRPGCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/DecalComponent.h"
#include "Components/HealthComponent.h"
#include "Components/CombatantComponent.h"
#include "Engine/World.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameModes/DroneRPGGameMode.h"
#include "LevelActors/RespawnPoint.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"
#include "Utilities/FunctionLibrary.h"
#include "Utilities/WeaponCreator.h"

ADroneRPGCharacter::ADroneRPGCharacter() : Super()
{
	// Set size for player capsule
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DroneMesh(TEXT("StaticMesh'/Game/TopDownCPP/Models/Drone.Drone'"));

	if (DroneMesh.Succeeded())
	{
		meshComponent->SetStaticMesh(DroneMesh.Object);
		meshComponent->SetupAttachment(RootComponent);
		meshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 8000;
	CameraBoom->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	TopDownCameraComponent->SetProjectionMode(ECameraProjectionMode::Orthographic);
	TopDownCameraComponent->SetOrthoWidth(10000);

	// Create a decal in the world to show the cursor's location
	CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/TopDownCPP/Blueprints/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}

	CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());
}

FColor ADroneRPGCharacter::GetTeamColour() {
	return GetGameMode()->GetTeamColour(GetTeam());
}

void ADroneRPGCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void ADroneRPGCharacter::Respawn()
{
	// Get our teams respawn point
	ARespawnPoint* respawn = GetRespawnPoint();

	GetCombatantComponent()->ResetCombatScore();

	// Did we find a respawn point?
	if (IsValid(respawn))
	{
		// Move us to the respawn point 
		SetActorLocation(respawn->GetSpawnLocation(), false, NULL, ETeleportType::ResetPhysics);

		// Fully Heal the drone
		GetHealthComponent()->FullHeal();
		GetCapsuleComponent()->SetGenerateOverlapEvents(true);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		meshComponent->SetHiddenInGame(false);
	}
}

ARespawnPoint* ADroneRPGCharacter::GetRespawnPoint()
{
	if (!IsValid(respawnPoint))
	{
		// Get all the respawn points
		TArray<ARespawnPoint*> respawnPoints = mGetActorsInWorld<ARespawnPoint>(GetWorld());

		for (ARespawnPoint* respawnPointFound : respawnPoints)
		{
			// Check if the respawn point belongs to our team 
			if (respawnPointFound->GetTeam() == GetTeam())
			{
				respawnPoint = respawnPointFound;
			}
		}
	}

	return respawnPoint;
}

void ADroneRPGCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	// Give each drone a random weapon
	EWeaponType type = UFunctionLibrary::GetRandomEnum<EWeaponType>(EWeaponType::End);
	SetWeapon(mGetDefaultWeapon(type, GetCombatantComponent()));

}

void ADroneRPGCharacter::UnitDied(UCombatantComponent* killer)
{
	Super::UnitDied(killer);
	mSetTimer(TimerHandle_Kill, &ADroneRPGCharacter::Respawn, 2.5f);
}
