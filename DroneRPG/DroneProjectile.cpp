// Fill out your copyright notice in the Description page of Project Settings.


#include "DroneProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DroneRPGCharacter.h"

// Sets default values
ADroneProjectile::ADroneProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Static reference to the mesh to use for the projectile
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ProjectileMeshAsset(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_NarrowCapsule.Shape_NarrowCapsule'"));

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
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f; // No gravity

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

// Called when the game starts or when spawned
void ADroneProjectile::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADroneProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADroneProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if (OtherActor != NULL && OtherActor != this && OtherActor != GetShooter() && OtherActor->GetClass() != ADroneProjectile::StaticClass())
	{
		ADroneRPGCharacter* target = Cast<ADroneRPGCharacter>(OtherActor);
		ADroneRPGCharacter* droneShooter = Cast<ADroneRPGCharacter>(GetShooter());

		// Did we hit a drone?
		if (target != NULL) {
			
			// Are we on a different team?
			if (target->GetTeam() != droneShooter->GetTeam()) {
				
				// Deal damage to enemy Drone
				target->RecieveHit(this);
				Destroy();
			}
			// We've hit an Ally!
			else {
				ProjectileMesh->IgnoreActorWhenMoving(target, true);
			}
		}
		// We've hit another projectile
		else if (OtherActor->IsA(ADroneProjectile::StaticClass())) {
			ProjectileMesh->IgnoreActorWhenMoving(OtherActor, true);
		}
	}
}

AActor* ADroneProjectile::GetShooter()
{
	return shooter;
}

void ADroneProjectile::SetShooter(AActor* val)
{
	shooter = val;
	ProjectileMesh->IgnoreActorWhenMoving(shooter, true);
}
