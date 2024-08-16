#include "Weapon/BulletProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ABulletProjectile::ABulletProjectile()
{
  PrimaryActorTick.bCanEverTick = true;

  CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
  CollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
  CollisionComponent->InitSphereRadius(1.0f);
  CollisionComponent->bReturnMaterialOnMove = true;
  CollisionComponent->OnComponentHit.AddDynamic(this, &ABulletProjectile::OnHit);

  ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
  ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
  ProjectileMovementComponent->InitialSpeed = 15000.0f;
  ProjectileMovementComponent->MaxSpeed = 15000.0f;
  ProjectileMovementComponent->bRotationFollowsVelocity = false;
  ProjectileMovementComponent->bShouldBounce = false;
  ProjectileMovementComponent->Bounciness = 0.0f;
  ProjectileMovementComponent->ProjectileGravityScale = 0.0f;

  RootComponent = CollisionComponent;
}

void ABulletProjectile::OnHit(UPrimitiveComponent * HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComponent, FVector NormalImpulse, const FHitResult & Hit)
{
  if (OtherActor)
  {
    UGameplayStatics::ApplyPointDamage(OtherActor, m_Damage, ProjectileMovementComponent->Velocity.GetSafeNormal(), Hit, GetInstigatorController(), GetOwner(), nullptr);
    OnObstacleHit(Hit);
  }

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

