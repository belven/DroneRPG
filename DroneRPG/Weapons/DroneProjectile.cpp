#include "DroneProjectile.h"
#include "../Plugins/FX/Niagara/Source/Niagara/Classes/NiagaraSystem.h"
#include "DroneRPG/Components/HealthComponent.h"
#include "DroneRPG/Utilities/FunctionLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include <DroneRPG/Components/CombatantComponent.h>
#include "DroneRPG/GameModes/DroneRPGGameMode.h"
#include "Kismet/GameplayStatics.h"

#define mSpawnSystemAttached(system, name) UNiagaraFunctionLibrary::SpawnSystemAttached(system, RootComponent, name, FVector(1), FRotator(1), EAttachLocation::SnapToTarget, false)

const float ADroneProjectile::Default_Initial_Speed = 7000.0f;
const float ADroneProjectile::Default_Initial_Lifespan = 1.2f;

// Sets default values
ADroneProjectile::ADroneProjectile()
{
#if WITH_EDITOR
	SetFolderPath(TEXT("Other/Projectiles"));
#endif
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ProjectileMeshAsset(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_NarrowCapsule.Shape_NarrowCapsule'"));
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> trailParticleSystem(TEXT("NiagaraSystem'/Game/TopDownCPP/ParticleEffects/TrailParticleSystem_2.TrailParticleSystem_2'"));

	if (trailParticleSystem.Succeeded())
	{
		trailSystem = trailParticleSystem.Object;
	}

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh0"));
	ProjectileMesh->SetStaticMesh(ProjectileMeshAsset.Object);
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->BodyInstance.SetCollisionProfileName("Projectile");
	ProjectileMesh->OnComponentHit.AddDynamic(this, &ADroneProjectile::OnHit);
	ProjectileMesh->CastShadow = false;

	RootComponent = ProjectileMesh;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement0"));
	ProjectileMovement->UpdatedComponent = ProjectileMesh;
	ProjectileMovement->InitialSpeed = Default_Initial_Speed;
	ProjectileMovement->MaxSpeed = Default_Initial_Speed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f; // No gravity

	InitialLifeSpan = Default_Initial_Lifespan;
	RootComponent->SetHiddenInGame(true);
}

void ADroneProjectile::BeginPlay()
{
	Super::BeginPlay();

	TArray<ADroneProjectile*> projectiles = mGetActorsInWorld<ADroneProjectile>(GetWorld());

	for (ADroneProjectile* projectile : projectiles)
	{
		IgnoreActor(projectile);
		projectile->IgnoreActor(this);
	}

	trialParticle = mSpawnSystemAttached(trailSystem, TEXT("trialParticle"));
}

void ADroneProjectile::IgnoreActor(AActor* actor)
{
	ProjectileMesh->IgnoreActorWhenMoving(actor, true);
}

void ADroneProjectile::SetTarget(FTargetData targetData)
{
	target = targetData;
}


void ADroneProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if (OtherActor != NULL && OtherActor != this && OtherActor != shooter->GetOwner() && OtherActor->GetClass() != StaticClass())
	{
		FTargetData targetData = mCreateTargetData(OtherActor);

		// Did we hit a drone?
		if (targetData.combatantComponent != NULL
			// Are we on a different team?
			&& targetData.GetTeam() != shooter->GetTeam())
		{
			// Deal damage to enemy Drone
			targetData.healthComponent->ReceiveDamage(GetDamage(), shooter);
			Destroy();
		}
		else
		{
			Destroy();
		}
	}
}

UCombatantComponent* ADroneProjectile::GetShooter()
{
	return shooter;
}

void ADroneProjectile::SetUpCollision()
{
	TArray<AActor*> actors = mGetActorsInWorld<AActor>(GetWorld());

	for (AActor* actor : actors)
	{
		UCombatantComponent* combatant = mGetCombatantComponent(actor);

		if (IsValid(combatant) && combatant->GetTeam() == shooter->GetTeam())
		{
			ProjectileMesh->IgnoreActorWhenMoving(combatant->GetOwner(), true);
		}
	}
}

void ADroneProjectile::SetShooter(UCombatantComponent* val)
{
	shooter = val;

	ADroneRPGGameMode* gameMode = Cast<ADroneRPGGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	trialParticle->SetColorParameter(TEXT("Beam Colour"), FLinearColor(gameMode->GetTeamColour(val->GetTeam())));
	SetUpCollision();
}