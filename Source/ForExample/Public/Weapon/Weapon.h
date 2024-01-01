#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class ABulletProjectile;
class AStaticMeshActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponAmmoSignature, int32, CurrentMagAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponShootSignature, FVector2D, Recoil);

UENUM(BlueprintType)
enum class EWeaponSound : uint8
{
  Shoot  UMETA(DisplayName="Shoot"),
  Reload UMETA(DisplayName="Reload"),
  NoAmmo UMETA(DisplayName="NoAmmo")
};

UCLASS(Abstract)
class FOREXAMPLE_API AWeapon : public AActor
{
  GENERATED_BODY()

public:

  AWeapon();

  void BeginPlay() override;

public:

  UFUNCTION(BlueprintCallable)
  virtual void StartShooting();

  UFUNCTION(BlueprintCallable)
  virtual void StopShooting();

  UFUNCTION(BlueprintCallable)
  virtual void Reload();

  UFUNCTION(BlueprintPure)
  bool IsReloading() const;

public: // Animation events

  UFUNCTION(BlueprintCallable, Category=Magazine)
  void OnMagazineExtracted();

  UFUNCTION(BlueprintCallable, Category=Magazine)
  void OnMagazineTaken();

  UFUNCTION(BlueprintCallable, Category=Magazine)
  void OnMagazineInserted();

protected:

  UFUNCTION(BlueprintImplementableEvent)
  void PlaySound(EWeaponSound Action);

  void ShootProjectile();

  void Shoot();

  bool IsAmmo() const;

  void OnNoAmmoLeft();

private:

  FVector2D GetRecoil() const;

  int GetDamage() const;

  bool CanBeShoot() const;

public:

  UPROPERTY(BlueprintAssignable, Category=Magazine)
  FWeaponAmmoSignature OnWeaponAmmoChanged;

  UPROPERTY(BlueprintAssignable, Category=Fire)
  FWeaponShootSignature OnWeaponShoot;

protected:

  UPROPERTY(EditDefaultsOnly, Category=Magazine)
  TSubclassOf<class AStaticMeshActor> MagazineClass;

  UPROPERTY(EditDefaultsOnly, Category=Magazine)
  int32 MagazineAmount = 0;

  UPROPERTY(BlueprintReadOnly, Category=Magazine)
  int32 CurrentMagazineAmount = 0;

  UPROPERTY(EditDefaultsOnly, Category=Shooting)
  float FireRate = 0.001f;

  UPROPERTY(EditDefaultsOnly, Category=Shooting)
  int BaseDamage = 0;

  UPROPERTY(EditDefaultsOnly, Category=Shooting)
  TArray<FVector2D> Recoil;

  UPROPERTY(EditDefaultsOnly, Category=Projectile)
  TSubclassOf<class ABulletProjectile> ProjectileClass;

  UPROPERTY(BlueprintReadWrite)
  UMeshComponent * ModelMesh = nullptr;

protected:

  static const int32 INVALID_AMMO_AMOUNT;

  AStaticMeshActor * m_MagazineActor = nullptr;
  FTimerHandle       m_ShootingTimer;

  bool m_IsShooting           = false;
  bool m_IsReloading          = false;
  bool m_NoAmmoEventTriggered = false;

};
