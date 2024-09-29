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

protected:

  void BeginPlay() override;

  UFUNCTION()
  void OnHit(UPrimitiveComponent * HitComponent, AActor * OtherActor, UPrimitiveComponent * OtherComponent, FVector NormalImpulse, const FHitResult & Hit);

  UFUNCTION(BlueprintImplementableEvent)
  void OnObstacleHit(const FHitResult & Hit);

  UFUNCTION(BlueprintImplementableEvent)
  void OnCharacterHit(const FHitResult & Hit);

  UFUNCTION(NetMulticast, Unreliable)
  void Multicast_OnObstacleHit(const FHitResult & Hit);

  UFUNCTION(NetMulticast, Unreliable)
  void Multicast_OnCharacterHit(const FHitResult & Hit);

protected:

  UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
  USphereComponent * CollisionComponent;

  UPROPERTY(VisibleAnywhere, Category=Movement)
  UProjectileMovementComponent * ProjectileMovementComponent;

};
