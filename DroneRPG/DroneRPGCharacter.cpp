#pragma once
#include "DroneRPGCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include <Kismet/GameplayStatics.h>
#include "Utilities/FunctionLibrary.h"
#include "NavigationSystem.h"
#include <Kismet/KismetSystemLibrary.h>
#include "DroneDamagerInterface.h"
#include "Controllers/DroneBaseAI.h"
#include "GameModes/DroneRPGGameMode.h"
#include "LevelActors/RespawnPoint.h"
#include "Weapons/Weapon.h"

ADroneRPGCharacter::ADroneRPGCharacter()
{
#if WITH_EDITOR
	SetFolderPath(TEXT("Characters"));
#endif
	team = 1;

	// Set size for player capsule
	const float capWidth = 120.0f;
	const float capHeight = 400.0f;

	GetCapsuleComponent()->InitCapsuleSize(capWidth, capHeight);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 150.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 6000.f;
	CameraBoom->SetRelativeRotation(FRotator(-75.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	TopDownCameraComponent->SetProjectionMode(ECameraProjectionMode::Perspective);
	TopDownCameraComponent->SetOrthoWidth(10000);

	// Create a decal in the world to show the cursor's location
	CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DroneMesh(TEXT("StaticMesh'/Game/TopDownCPP/Models/Drone.Drone'"));

	meshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DroneMesh"));
	if (DroneMesh.Succeeded())
	{
		meshComponent->SetStaticMesh(DroneMesh.Object);
		meshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		meshComponent->SetRelativeLocation(FVector(40, -25, 100));
		meshComponent->SetupAttachment(RootComponent);
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/TopDownCPP/Blueprints/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}

	CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	GetCharacterMovement()->MaxWalkSpeed = 1500;

	healthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

	healthComponent->OnUnitDied.AddUniqueDynamic(this, &ADroneRPGCharacter::KillDrone);
}

void ADroneRPGCharacter::BeginDestroy()
{
	Super::BeginDestroy();
}

void ADroneRPGCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (!GetGameMode()->GetDrones().Contains(this))
	{
		GetGameMode()->GetDrones().Add(this);
	}
}

FColor ADroneRPGCharacter::GetTeamColour() {
	return UFunctionLibrary::GetTeamColour(GetTeam());
}

void ADroneRPGCharacter::SetUpDrone()
{
	kills = 0;
	deaths = 0;

	// Give each drone a random weapon
	EWeaponType type = UFunctionLibrary::GetRandomEnum<EWeaponType>(EWeaponType::End);
	SetWeapon(mGetDefaultWeapon(type, this));
}

void ADroneRPGCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	SetUpDrone();
}

void ADroneRPGCharacter::SetTeam(int32 val)
{
	team = val;
	healthComponent->SetTeamColour(GetTeamColour());
}

ADroneRPGGameMode* ADroneRPGCharacter::GetGameMode()
{
	if (!IsValid(gameMode))
	{
		gameMode = Cast<ADroneRPGGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	}
	return gameMode;
}

void ADroneRPGCharacter::Respawn() {
	// Get our teams respawn point
	ARespawnPoint* respawn = GetRespawnPoint();

	// Did we find a respawn point?
	if (respawn != NULL) {
		// Move us to the respawn point 
		FNavLocation loc;
		mRandomPointInNavigableRadius(respawn->GetActorLocation(), respawn->GetSize(), loc);
		SetActorLocation(loc, false, NULL, ETeleportType::ResetPhysics);

		// Fully Heal the drone
		healthComponent->FullHeal();

		meshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		meshComponent->SetHiddenInGame(false);
	}
}

ARespawnPoint* ADroneRPGCharacter::GetRespawnPoint()
{
	if (!IsValid(respawnPoint)) {
		// Get all the respawn points
		TArray<ARespawnPoint*> respawnPoints = mGetActorsInWorld<ARespawnPoint>(GetWorld());

		for (ARespawnPoint* respawnPointFound : respawnPoints) {
			// Check if the respawn point belongs to our team 
			if (respawnPointFound->GetTeam() == team) {
				respawnPoint = respawnPointFound;
			}
		}
	}

	return respawnPoint;
}

void ADroneRPGCharacter::KillDrone(AActor* killer)
{
	deaths++;
	meshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	meshComponent->SetHiddenInGame(true);

	mSetTimer(TimerHandle_Kill, &ADroneRPGCharacter::Respawn, 1.5f);

	// Check if the killer uses UDroneDamagerInterface
	if (mImplements(killer, UDroneDamagerInterface)) {
		IDroneDamagerInterface* damageDealer = Cast<IDroneDamagerInterface>(killer);

		// Tell the killer they've killed us
		damageDealer->DroneKilled(this);

		// Tell the gamemode we've died, to update score etc.
		GetGameMode()->EntityKilled(this, killer);

		// Add text to the kill feed
		// TODO Move this into gamemode and make a log of kills, maybe with a rolling kill feed.
		TArray< FStringFormatArg > args;
		args.Add(FStringFormatArg(GetDroneName()));
		args.Add(FStringFormatArg(damageDealer->GetDamagerName()));

		GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::White, FString::Format(TEXT("Drone {0} was killed by a {1}"), args));
	}
}

FString ADroneRPGCharacter::GetDroneName()
{	
	if (droneName.IsEmpty() && GetGameMode()->GetDrones().Contains(this)) {
		droneName = FString::FromInt(GetGameMode()->GetDrones().IndexOfByKey(this));
	}
	return droneName;
}