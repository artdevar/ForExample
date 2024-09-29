#include "Weapon/BulletProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/Weapon.h"

ABulletProjectile::ABulletProjectile()
{
  PrimaryActorTick.bCanEverTick = true;
  bReplicates = true;

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

  CollisionComponent->IgnoreActorWhenMoving(GetOwner(),             true); // Weapon
  CollisionComponent->IgnoreActorWhenMoving(GetOwner()->GetOwner(), true); // Weapon carrier

  SetLifeSpan(15);
}

void ABulletProjectile::SetDirection(const FVector & Direction)
{
  ProjectileMovementComponent->Velocity = ProjectileMovementComponent->InitialSpeed * Direction;
}

void ABulletProjectile::OnHit(UPrimitiveComponent * HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComponent, FVector NormalImpulse, const FHitResult & Hit)
{
  if (!HasAuthority())
  {
    Destroy();
    return;
  }

  const bool    IsCharacterHit = OtherActor && OtherActor->IsA<ACharacter>();
  const auto    Weapon         = GetOwner<AWeapon>();
  const int32   Damage         = Weapon->GetDamage(Hit.PhysMaterial.Get());
  const FVector HitFrom        = ProjectileMovementComponent->Velocity.GetSafeNormal();

  USoundBase * HitSound = Weapon->GetSound(IsCharacterHit ? EWeaponSound::BodyHit : EWeaponSound::ObstacleHit);

  UGameplayStatics::ApplyPointDamage(OtherActor, Damage, HitFrom, Hit, GetInstigatorController(), GetOwner(), UBulletDamageType::StaticClass());
  UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, Hit.ImpactPoint);

  if (IsCharacterHit)
    Multicast_OnCharacterHit(Hit);
  else
    Multicast_OnObstacleHit(Hit);

  SetActorHiddenInGame(true);
  SetActorEnableCollision(false);
  SetActorTickEnabled(false);

  auto LogStr = FString::Format(TEXT("HIT INFO: {0}; {1}; {2}."), {Hit.PhysMaterial->GetName(), OtherActor->GetDebugName(OtherActor), Damage});
  GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Yellow, LogStr, false);
}

void ABulletProjectile::Multicast_OnObstacleHit_Implementation(const FHitResult & Hit)
{
  if (!HasAuthority())
    OnObstacleHit(Hit);
}

void ABulletProjectile::Multicast_OnCharacterHit_Implementation(const FHitResult & Hit)
{
  if (!HasAuthority())
    OnCharacterHit(Hit);
}