#include "DroneProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "Components/SphereComponent.h"
#include "DroneRPG/GameModes/DroneRPGGameMode.h"
#include "Kismet/GameplayStatics.h"

#define mSpawnSystemAttached(system, name) UNiagaraFunctionLibrary::SpawnSystemAttached(system, RootComponent, name, FVector(1), FRotator(1), EAttachLocation::SnapToTarget, false)

const float ADroneProjectile::Default_Initial_Speed = 7000.0f;
const float ADroneProjectile::Default_Initial_Lifespan = 1.2f;

// Sets default values
ADroneProjectile::ADroneProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> trailParticleSystem(TEXT("NiagaraSystem'/Game/TopDownCPP/ParticleEffects/TrailParticleSystem_2.TrailParticleSystem_2'"));

	if (trailParticleSystem.Succeeded())
	{
		trailSystem = trailParticleSystem.Object;
	}

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->SetCollisionProfileName("Projectile");
	CollisionComp->SetGenerateOverlapEvents(true);
	RootComponent = CollisionComp;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement0"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = Default_Initial_Speed;
	ProjectileMovement->MaxSpeed = Default_Initial_Speed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f; // No gravity
	ProjectileMovement->bSweepCollision = true;
	ProjectileMovement->Friction = 0;

	InitialLifeSpan = Default_Initial_Lifespan;
}

void ADroneProjectile::BeginPlay()
{
	Super::BeginPlay();
#if WITH_EDITOR
	SetFolderPath(TEXT("Other/Projectiles"));
#endif

	trialParticle = mSpawnSystemAttached(trailSystem, TEXT("trialParticle"));
	CollisionComp->SetSphereRadius(200, true);
}


void ADroneProjectile::SetTarget(FCombatantData targetData)
{
	target = targetData;
}

void ADroneProjectile::HItValidTarget(const FCombatantData& targetData)
{
	targetData.healthComponent->ReceiveDamage(GetDamage(), shooter);
}

bool ADroneProjectile::CheckIfValidTarget(const FCombatantData& targetData)
{
	return targetData.IsValid() && targetData.IsAlive() && targetData.GetTeam() != GetShooter()->GetTeam();
}

void ADroneProjectile::ActorDetected(AActor* OtherActor)
{
	bool isProjectile = OtherActor->GetClass() == StaticClass();

	bool isObjective = OtherActor->Tags.Contains("Objective");
	// Only add impulse and destroy projectile if we hit a physics
	if (!isProjectile && !isObjective)
	{
		FCombatantData targetData = mCreateCombatantData(OtherActor);

		// Did we hit a valid target?
		if (CheckIfValidTarget(targetData))
		{
			// Deal damage to enemy Drone
			HItValidTarget(targetData);
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

void ADroneProjectile::OnBaseProjectileOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ActorDetected(OtherActor);
}

void ADroneProjectile::SetShooter(UCombatantComponent* val)
{
	shooter = val;

	ADroneRPGGameMode* gameMode = Cast<ADroneRPGGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	trialParticle->SetColorParameter(TEXT("Beam Colour"), FLinearColor(gameMode->GetTeamColour(val->GetTeam())));
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ADroneProjectile::OnBaseProjectileOverlap);
}