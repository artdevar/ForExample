#include "Weapon/BulletProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "DrawDebugHelpers.h"

ABulletProjectile::ABulletProjectile()
{
  PrimaryActorTick.bCanEverTick = true;

  if (!CollisionComponent)
  {
    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
    CollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
    CollisionComponent->InitSphereRadius(1.0f);
    CollisionComponent->OnComponentHit.AddDynamic(this, &ABulletProjectile::OnHit);
    RootComponent = CollisionComponent;
  }

  if (!ProjectileMovementComponent)
  {
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
    ProjectileMovementComponent->InitialSpeed = 15000.0f;
    ProjectileMovementComponent->MaxSpeed = 15000.0f;
    ProjectileMovementComponent->bRotationFollowsVelocity = false;
    ProjectileMovementComponent->bShouldBounce = false;
    ProjectileMovementComponent->Bounciness = 0.0f;
    ProjectileMovementComponent->ProjectileGravityScale = 0.0f;
  }
}

void ABulletProjectile::OnHit(UPrimitiveComponent * HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComponent, FVector NormalImpulse, const FHitResult & Hit)
{
  DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 4, 12, FColor::Red, false, 5);

  if (OtherActor)
    UGameplayStatics::ApplyPointDamage(OtherActor, m_Damage, ProjectileMovementComponent->Velocity.GetSafeNormal(), Hit, GetInstigatorController(), GetOwner(), nullptr);

  Destroy();
}

void ABulletProjectile::SetDirection(const FVector & Direction)
{
  ProjectileMovementComponent->Velocity = ProjectileMovementComponent->InitialSpeed * Direction;
}

void ABulletProjectile::SetDamage(int Damage)
{
  m_Damage = Damage;
}

