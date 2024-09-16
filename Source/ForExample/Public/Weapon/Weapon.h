#pragma once

#include "CoreMinimal.h"
#include "Components/InteractableActor.h"
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

  USoundBase * GetSound(EWeaponSound SoundType) const;

  UFUNCTION(BlueprintCallable)
  void SwitchFireMode();

  int32 GetDamage(const TSoftObjectPtr<class UPhysicalMaterial> & MaterialHit) const;

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

  virtual void InitFromTable();

private:

  FWeaponRecoilParams GetRecoil() const;

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

  UPROPERTY(EditDefaultsOnly, meta=(RowType=WeaponData), Category=Data)
  FDataTableRowHandle WeaponDataHandle;

  UPROPERTY(EditDefaultsOnly, Category = Projectile)
  TSubclassOf<class ABulletProjectile> ProjectileClass;

  UPROPERTY(EditDefaultsOnly, Category=Magazine)
  TSubclassOf<class AStaticMeshActor> MagazineClass;

  UPROPERTY(BlueprintReadOnly, Category = Magazine)
  int32 CurrentMagazineAmount = 0;

  UPROPERTY(BlueprintReadWrite)
  UMeshComponent * ModelMesh = nullptr;

  UPROPERTY(BlueprintReadOnly)
  EWeaponFireMode FireMode = EWeaponFireMode::Single;

protected: // Data table

  TMap<EWeaponSound, FSoundsSet>                 * Sounds         = nullptr;
  TMap<TSoftObjectPtr<UPhysicalMaterial>, int32> * HitboxDamage   = nullptr;
  TArray<EWeaponFireMode>                        * FireModes      = nullptr;
  FWeaponRecoilParams                            * RecoilParams   = nullptr;
  float                                            FireRate       = 0.1f;
  int32                                            MagazineAmount = 0;

protected:

  static const int32 INVALID_AMMO_AMOUNT;

  AStaticMeshActor * m_MagazineActor = nullptr;
  FTimerHandle       m_ShootingTimer;

  bool m_IsShooting  = false;
  bool m_IsReloading = false;

  std::optional<uint32> m_AmountAmmoToShoot;

};
