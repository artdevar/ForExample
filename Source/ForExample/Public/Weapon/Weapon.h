#pragma once

#include "CoreMinimal.h"
#include "Components/InteractableActor.h"
#include "GameFramework/Actor.h"
#include "Shared/Weapons.h"
#include <optional>
#include "Weapon.generated.h"

class UMyGameInstance;
class ABulletProjectile;
class AStaticMeshActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponAmmoSignature, int32, CurrentMagAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponShootSignature, FWeaponRecoilParams, RecoilParams);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponFireModeSignature, EWeaponFireMode, FireMode);

UCLASS(Abstract)
class FOREXAMPLE_API AWeapon : public AInteractableActor
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

  UFUNCTION(BlueprintCallable)
  void PlaySound(EWeaponSound SoundType);

  UFUNCTION(BlueprintCallable)
  void SwitchFireMode();

public: // Animation events

  UFUNCTION(BlueprintCallable, Category=Magazine)
  void OnMagazineExtracted();

  UFUNCTION(BlueprintCallable, Category=Magazine)
  void OnMagazineTaken();

  UFUNCTION(BlueprintCallable, Category=Magazine)
  void OnMagazineInserted();

protected:

  UFUNCTION(BlueprintImplementableEvent)
  void SpawnShootProjectile();

  void ShootProjectile();

  void Shoot();

  bool IsAmmo() const;

  void OnNoAmmoLeft();

private:

  FWeaponRecoilParams GetRecoil() const;

  int GetDamage() const;

  bool CanBeShoot() const;

  void ApplyFireModeAmmoLimit();

  void ResetFireModeAmmoLimit();

public:

  UPROPERTY(BlueprintAssignable, Category=Magazine)
  FWeaponAmmoSignature OnWeaponAmmoChanged;

  UPROPERTY(BlueprintAssignable, Category=Shooting)
  FWeaponShootSignature OnWeaponShoot;

  UPROPERTY(BlueprintAssignable, Category=Shooting)
  FWeaponFireModeSignature OnFireModeChanged;

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
  FWeaponRecoilParams RecoilParams;

  UPROPERTY(EditDefaultsOnly, Category=Shooting)
  TArray<EWeaponFireMode> FireModes;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Shooting)
  EWeaponFireMode FireMode = EWeaponFireMode::Single;

  UPROPERTY(EditDefaultsOnly, Category=Projectile)
  TSubclassOf<class ABulletProjectile> ProjectileClass;

  UPROPERTY(BlueprintReadWrite)
  UMeshComponent * ModelMesh = nullptr;

  UPROPERTY(BlueprintReadOnly)
  UMyGameInstance * GameInstance = nullptr;

protected:

  static const int32 INVALID_AMMO_AMOUNT;

  AStaticMeshActor * m_MagazineActor = nullptr;
  FTimerHandle       m_ShootingTimer;

  bool m_IsShooting  = false;
  bool m_IsReloading = false;

  std::optional<uint32> m_AmountAmmoToShoot;

};
