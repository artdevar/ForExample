#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class FOREXAMPLE_API ABulletProjectile : public AActor
{
  GENERATED_BODY()

public:

  ABulletProjectile();

  void SetDirection(const FVector & Direction);

  void SetDamage(int Damage);

protected:

  UFUNCTION()
  void OnHit(UPrimitiveComponent * HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComponent, FVector NormalImpulse, const FHitResult & Hit);

  // Implement this in cpp
  UFUNCTION(BlueprintImplementableEvent)
  void OnObstacleHit(FHitResult Hit);

protected:

  UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
  USphereComponent * CollisionComponent;

  UPROPERTY(VisibleAnywhere, Category=Movement)
  UProjectileMovementComponent * ProjectileMovementComponent;

  int m_Damage;

};
