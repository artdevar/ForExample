#include "Weapon/BulletProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/Weapon.h"

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
  ProjectileMovementComponent->MaxSpeed                 = 20000.0f;
  ProjectileMovementComponent->InitialSpeed             = 20000.0f;
  ProjectileMovementComponent->bRotationFollowsVelocity = false;
  ProjectileMovementComponent->bShouldBounce            = false;
  ProjectileMovementComponent->Bounciness               = 0.0f;
  ProjectileMovementComponent->ProjectileGravityScale   = 0.1f;

  RootComponent = CollisionComponent;
}

void ABulletProjectile::BeginPlay()
{
  Super::BeginPlay();

  SetLifeSpan(10);
}

void ABulletProjectile::OnHit(UPrimitiveComponent * HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComponent, FVector NormalImpulse, const FHitResult & Hit)
{
  const bool    IsCharacterHit = OtherActor && OtherActor->IsA<ACharacter>();
  const auto    Weapon         = GetOwner<AWeapon>();
  const int32   Damage         = Weapon->GetDamage(Hit.PhysMaterial.Get());
  const FVector HitFrom        = ProjectileMovementComponent->Velocity.GetSafeNormal();

  USoundBase * HitSound = Weapon->GetSound(IsCharacterHit ? EWeaponSound::BodyHit : EWeaponSound::ObstacleHit);

  if (IsCharacterHit)
    OnCharacterHit(Hit);
  else
    OnObstacleHit(Hit);

  UGameplayStatics::ApplyPointDamage(OtherActor, Damage, HitFrom, Hit, GetInstigatorController(), GetOwner(), UBulletDamageType::StaticClass());
  UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, Hit.ImpactPoint);

  GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Blue, Hit.PhysMaterial->GetName(), false);

  Destroy();
}

void ABulletProjectile::SetDirection(const FVector & Direction)
{
  ProjectileMovementComponent->Velocity = ProjectileMovementComponent->InitialSpeed * Direction;
}