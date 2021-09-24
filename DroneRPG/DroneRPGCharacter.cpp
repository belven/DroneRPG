#include "DroneRPGCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include <Kismet/GameplayStatics.h>
#include <Kismet/KismetMathLibrary.h>
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "../Plugins/FX/Niagara/Source/Niagara/Classes/NiagaraSystem.h"
#include "DroneProjectile.h"
#include "DroneBaseAI.h"
#include "FunctionLibrary.h"
#include "RespawnPoint.h"
#include "NavigationSystem.h"
#include "Weapon.h"
#include "Shotgun.h"
#include "Materials/MaterialInstanceConstant.h"
#include <Materials/MaterialLayersFunctions.h>

#define mSpawnSystemAttached(system, name) UNiagaraFunctionLibrary::SpawnSystemAttached(system, meshComponent, name, FVector(1), FRotator(1), EAttachLocation::SnapToTarget, false)

ADroneRPGCharacter::ADroneRPGCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(120.0f, 250.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; // Rotate character to moving direction
	//GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 150.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> auraParticleSystem(TEXT("/Game/TopDownCPP/ParticleEffects/AuraSystem"));

	if (auraParticleSystem.Succeeded()) {
		auraSystem = auraParticleSystem.Object;
	}

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
		meshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		meshComponent->SetRelativeLocation(FVector(40, -25, 100));
		meshComponent->SetupAttachment(RootComponent);
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> shieldIsntance(TEXT("MaterialInstanceConstant'/Game/TopDownCPP/Materials/Shield_Inst.Shield_Inst'"));

	if (shieldIsntance.Succeeded()) {
		matInstanceConst = shieldIsntance.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/TopDownCPP/Blueprints/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}

	CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SetDefaults();

	energyRegen = 10;
	shieldRegen = 10;

	shieldRegenDelay = 3.0f;
	energyRegenDelay = 3.0f;

	smallShieldExp = 40;
	largeShieldExp = 20;
	maxWipe = 0.2;
	minWipe = -0.2;
	wipeValue = FMath::RandRange(minWipe, maxWipe);

	healthParticleSize = 20;

	SetTeam(1);
	GetCharacterMovement()->MaxWalkSpeed = 1500;

	if (!UFunctionLibrary::GetDrones().Contains(this))
		UFunctionLibrary::GetDrones().Add(this);
}

void ADroneRPGCharacter::BeginDestroy()
{
	Super::BeginDestroy();

	if (UFunctionLibrary::GetDrones().Contains(this))
		UFunctionLibrary::GetDrones().Remove(this);
}

void ADroneRPGCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Set up particle effect defaults
	healthParticle = mSpawnSystemAttached(auraSystem, TEXT("healthParticle"));
	healthParticle->SetFloatParameter(TEXT("Radius"), 125);
	healthParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Green));
	healthParticle->SetFloatParameter(TEXT("Size"), healthParticleSize);


	shieldMesh = LoadObject<UStaticMesh>(this, TEXT("StaticMesh'/Game/TopDownCPP/Models/Shield.Shield'"));
	//shieldMesh = LoadObject<UStaticMesh>(this, TEXT("StaticMesh'/Game/TopDownCPP/Models/Shape_Sphere'"));

	if (shieldMesh != NULL) {
		FColor col = GetTeamColour();
		shieldMeshComp = NewObject<UInstancedStaticMeshComponent>(this);
		shieldMeshComp->SetWorldScale3D(FVector(1));
		shieldMeshComp->SetStaticMesh(shieldMesh);
		shieldMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		shieldMeshComp->SetupAttachment(RootComponent);
		shieldMeshComp->RegisterComponent();

		FTransform trans = GetActorTransform();
		trans.AddToTranslation(FVector(0, 0, 30));
		meshIndex = shieldMeshComp->AddInstanceWorldSpace(trans);

		FLinearColor col2 = FLinearColor(col);
		col2.R *= 300;
		col2.G *= 300;
		col2.B *= 300;
		col2.A = 0;

		SetMaterialFloat(TEXT("Wipe"), minWipe);
		SetMaterialFloat(TEXT("Exp"), largeShieldExp);
		SetMaterialColour(TEXT("Emissive Color"), col2);
	}

	kills = 0;
	deaths = 0;
	EWeaponType type = UFunctionLibrary::GetRandomEnum<EWeaponType>(EWeaponType::End);
	SetWeapon(mGetDefaultWeapon(type, this));

	Respawn();
}

void ADroneRPGCharacter::PulseShield() {
	const float increment = 0.02;

	wipeValue += increment;

	if (wipeValue >= maxWipe)
		wipeValue = minWipe;

	SetMaterialFloat(TEXT("Wipe"), wipeValue);
}

void ADroneRPGCharacter::SetDefaults() {
	float energy = 150;
	float health = 150;
	float shields = 150;

	maxStats.energy = energy;
	maxStats.health = health;
	maxStats.shields = shields;

	currentStats.energy = energy;
	currentStats.health = health;
	currentStats.shields = shields;

	canRegenShields = true;
	shieldsCritical = false;
	healthStatus = FColor::Green;
	shieldsActive = true;
}

FColor ADroneRPGCharacter::GetTeamColour() {
	return *UFunctionLibrary::GetTeamColours().Find(GetTeam());
}

bool ADroneRPGCharacter::IsHealthy() {
	return healthStatus == FColor::Green;
}

float ADroneRPGCharacter::ClampValue(float value, float max, float min) {
	if (value < min)
		value = min;

	if (value > max)
		value = max;

	return value;
}

void ADroneRPGCharacter::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Check if we have a drone and we don't already have it in the list
	if (mIsA(OtherActor, ADroneRPGCharacter) && !dronesInArea.Contains(OtherActor)) {

		// Add it to the list and re-calculate ownership
		dronesInArea.Add(Cast<ADroneRPGCharacter>(OtherActor));
	}
}

void ADroneRPGCharacter::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// Check if we have a drone and we have it in the list
	if (mIsA(OtherActor, ADroneRPGCharacter) && dronesInArea.Contains(OtherActor)) {

		// Remove it from the list and re-calculate ownership
		dronesInArea.Remove(Cast<ADroneRPGCharacter>(OtherActor));
	}
}

void ADroneRPGCharacter::Respawn() {
	// Get our teams respawn point
	ARespawnPoint* respawn = GetRespawnPoint();

	// Did we find a respawn point?
	if (respawn != NULL) {
		// Move us to the respawn point 
		FNavLocation loc;
		mRandomReachablePointInRadius(respawn->GetActorLocation(), 1000.0f, loc);
		SetActorLocation(loc);

		// Fully Heal the drone
		FullHeal();

		meshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		meshComponent->SetHiddenInGame(false);
	}
}

ARespawnPoint* ADroneRPGCharacter::GetRespawnPoint()
{
	// Get all the respawn points
	TArray<ARespawnPoint*> respawnPoints = mGetActorsInWorld<ARespawnPoint>(GetWorld());

	for (ARespawnPoint* respawnPoint : respawnPoints) {
		// Check if the respawn point belongs to our team 
		if (respawnPoint->GetTeam() == team) {
			return respawnPoint;
		}
	}

	return NULL;
}

void ADroneRPGCharacter::KillDrone() {
	if (DroneDied.IsBound())
		DroneDied.Broadcast(this);

	// TODO: set up the concept of respawning the player and making a spectator mode whilst that's happening
	currentStats.health = 0;
	currentStats.shields = 0;
	canRegenShields = false;

	meshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	meshComponent->SetHiddenInGame(true);

	//shieldParticle->SetHiddenInGame(true);
	healthParticle->SetHiddenInGame(true);

	mSetTimer(TimerHandle_Kill, &ADroneRPGCharacter::Respawn, 1.5f);
}

void ADroneRPGCharacter::RecieveHit(ADroneProjectile* projectile) {
	float damage = projectile->GetDamage();

	// Disable our shield regen as we've been hit
	canRegenShields = false;
	mSetTimer(TimerHandle_ShieldRegenRestart, &ADroneRPGCharacter::StartShieldRegen, shieldRegenDelay);

	// Take damage to shields
	if (HasShields()) {

		// We have less shields than damage, so remove are shields from the damage, to allow us to take it as health
		if (currentStats.shields < damage) {
			damage -= currentStats.shields;
			currentStats.shields = 0;
		}
		// We have more shields than damage dealt, so take it all to shields
		else {
			currentStats.shields -= damage;
		}

		// Weaken max shields, to prevent ships being as strong for the whole match
		if (maxStats.shields > 50) {

			// Take half damage taken away from max shields TODO work out a decent value for this, change it based on projectile?
			maxStats.shields -= (damage * 0.5);
			ClampValue(maxStats.shields, maxStats.shields, 50);
		}
		CalculateShieldParticles();
	}

	// If we have 0 shields, then take health damage
	if (currentStats.shields <= 0) {
		currentStats.health -= damage;

		// If we have no health, kill the character 
		if (currentStats.health <= 0) {
			deaths++;
			ADroneRPGCharacter* attacker = Cast<ADroneRPGCharacter>(projectile->GetShooter());
			attacker->SetKills(attacker->GetKills() + 1);
			KillDrone();
		}

		CalculateHealthColours();
	}

	// Inform our controller that we've been hit, only the AI version needs to know for now, so it can respond to combat
	if (GetController() != NULL && mIsA(GetController(), ADroneBaseAI)) {
		ADroneBaseAI* con = Cast<ADroneBaseAI>(GetController());
		con->DroneAttacked(projectile->GetShooter());
	}

	ClampValue(currentStats.health, maxStats.health, 0);
	ClampValue(currentStats.shields, maxStats.shields, 0);
}

bool ADroneRPGCharacter::IsAlive()
{
	return currentStats.health > 0;
}

void ADroneRPGCharacter::FullHeal() {
	SetDefaults();
	healthParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Green));

	SetMaterialFloat(TEXT("Exp"), largeShieldExp);

	shieldMeshComp->SetHiddenInGame(false);
	healthParticle->SetHiddenInGame(false);
}

void ADroneRPGCharacter::CalculateHealthColours() {
	// Update the colours, above 70% Green, above 45% Orange and below 45% Red
	if (currentStats.health < (maxStats.health * 0.45f) && healthStatus != FColor::Red) {
		healthParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Red));
		healthStatus = FColor::Red;
	}
	else if (currentStats.health < (maxStats.health * 0.7f) && healthStatus != FColor::Orange && healthStatus != FColor::Red) {
		healthParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Orange));
		healthStatus = FColor::Orange;
	}
	else if (currentStats.health > (maxStats.health * 0.7f) && healthStatus != FColor::Green) {
		healthParticle->SetColorParameter(TEXT("Base Colour"), FLinearColor(FColor::Green));
		healthStatus = FColor::Green;
	}
}

bool ADroneRPGCharacter::HasShields() {
	return currentStats.shields > 0;
}

void ADroneRPGCharacter::CalculateShieldParticles() {
	// Change the colour and size of the particles base on shield value, they'll be smaller and darker if we have < 50% shields
	if (currentStats.shields < (maxStats.shields * 0.5f) && !shieldsCritical) {
		SetMaterialFloat(TEXT("Exp"), smallShieldExp);
		shieldsCritical = true;
	}
	else if (currentStats.shields > (maxStats.shields * 0.5f) && shieldsCritical) {
		SetMaterialFloat(TEXT("Exp"), largeShieldExp);
		shieldsCritical = false;
	}

	// If we have 0 shields, disable the particle effect
	if (currentStats.shields <= 0 && shieldsActive) {
		shieldMeshComp->SetHiddenInGame(true);
		shieldsActive = false;
	}
	else if (currentStats.shields > 0 && !shieldsActive) {
		shieldMeshComp->SetHiddenInGame(false);
		shieldsActive = true;
	}
}

void ADroneRPGCharacter::SetMaterialColour(FName param, FLinearColor value)
{
	shieldMeshComp->SetVectorParameterValueOnMaterials(param, FVector(value.R, value.G, value.B));
}

void ADroneRPGCharacter::SetMaterialFloat(FName param, float value)
{
	shieldMeshComp->SetScalarParameterValueOnMaterials(param, value);
}

void ADroneRPGCharacter::CalculateShields(float DeltaSeconds) {
	// If our shields aren't at max and we haven't recently been hit, regen our shields
	if (currentStats.shields < maxStats.shields && canRegenShields) {
		float value = shieldRegen * DeltaSeconds;

		// Do we have the energy to regen our shields? TODO: show how represent energy as a particle etc. Do we want to make energy regen stop when used?
		if (currentStats.energy > value) {
			currentStats.shields += value;
			currentStats.energy -= value;

			ClampValue(currentStats.shields, maxStats.shields, 0);
			ClampValue(currentStats.energy, maxStats.energy, 0);
		}
		CalculateShieldParticles();
	}
}

void ADroneRPGCharacter::CalculateEnergy(float DeltaSeconds) {
	// If our current energy isn't at max, restore some energy
	if (currentStats.energy < maxStats.energy) {
		float value = energyRegen * DeltaSeconds;
		currentStats.energy += value;

		ClampValue(currentStats.energy, maxStats.energy, 0);
	}
}

void ADroneRPGCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	PulseShield();

	CalculateEnergy(DeltaSeconds);
	CalculateShields(DeltaSeconds);
}

void ADroneRPGCharacter::StartShieldRegen()
{
	canRegenShields = true;
}