// Fill out your copyright notice in the Description page of Project Settings.


#include "DroneProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DroneRPGCharacter.h"
#include "FunctionLibrary.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "../Plugins/FX/Niagara/Source/Niagara/Classes/NiagaraSystem.h"

#define mSpawnSystemAttached(system, name) UNiagaraFunctionLibrary::SpawnSystemAttached(system, RootComponent, name, FVector(1), FRotator(1), EAttachLocation::SnapToTarget, false)

// Sets default values
ADroneProjectile::ADroneProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Static reference to the mesh to use for the projectile
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ProjectileMeshAsset(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_NarrowCapsule.Shape_NarrowCapsule'"));

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem>trailParticleSystem(TEXT("NiagaraSystem'/Game/TopDownCPP/ParticleEffects/TrailParticleSystem_2.TrailParticleSystem_2'"));

	if (trailParticleSystem.Succeeded()) {
		trailSystem = trailParticleSystem.Object;
	}

	// Create mesh component for the projectile sphere
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh0"));
	ProjectileMesh->SetStaticMesh(ProjectileMeshAsset.Object);
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->BodyInstance.SetCollisionProfileName("Projectile");
	ProjectileMesh->OnComponentHit.AddDynamic(this, &ADroneProjectile::OnHit);		// set up a notification for when this component hits something
	ProjectileMesh->CastShadow = false;
	//ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	RootComponent = ProjectileMesh;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement0"));
	ProjectileMovement->UpdatedComponent = ProjectileMesh;
	ProjectileMovement->InitialSpeed = 5000.f;
	ProjectileMovement->MaxSpeed = 5000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f; // No gravity

	InitialLifeSpan = 2.0f;
	RootComponent->SetHiddenInGame(true);
}

// Called when the game starts or when spawned
void ADroneProjectile::BeginPlay()
{
	Super::BeginPlay();

	TArray<ADroneProjectile*> projectiles = mGetActorsInWorld<ADroneProjectile>(GetWorld());

	for (ADroneProjectile* projectile : projectiles) {
		IgnoreActor(projectile);
		projectile->IgnoreActor(this);
	}

	trialParticle = mSpawnSystemAttached(trailSystem, TEXT("trialParticle"));

}

void ADroneProjectile::IgnoreActor(AActor* actor) {
	ProjectileMesh->IgnoreActorWhenMoving(actor, true);
}

// Called every frame
void ADroneProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//ADroneRPGCharacter* target;
	//lookAt = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), target->GetActorLocation());
	//lookAt.Pitch = mDroneRotation.Pitch;
	//lookAt.Roll = mDroneRotation.Roll;
	//SetActorRotation(lookAt); 
}

void ADroneProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if (OtherActor != NULL && OtherActor != this && OtherActor != shooter && OtherActor->GetClass() != ADroneProjectile::StaticClass())
	{
		ADroneRPGCharacter* target = Cast<ADroneRPGCharacter>(OtherActor);

		// Did we hit a drone?
		if (target != NULL) {

			// Are we on a different team?
			if (target->GetTeam() != shooter->GetTeam()) {

				// Deal damage to enemy Drone
				target->RecieveHit(this);
				Destroy();
			}
		}
		else {
			Destroy();
		}
	}
}

ADroneRPGCharacter* ADroneProjectile::GetShooter()
{
	return shooter;
}

void ADroneProjectile::SetUpCollision() {
	TArray<ADroneRPGCharacter*> drones = mGetActorsInWorld<ADroneRPGCharacter>(GetWorld());

	for (ADroneRPGCharacter* drone : drones) {
		if (drone->GetTeam() == shooter->GetTeam()) {
			ProjectileMesh->IgnoreActorWhenMoving(drone, true);
		}
	}
}

void ADroneProjectile::SetShooter(ADroneRPGCharacter* val)
{
	shooter = val;
	trialParticle->SetColorParameter(TEXT("Beam Colour"), FLinearColor(GetShooter()->GetTeamColour()));
	SetUpCollision();
}
