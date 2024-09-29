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

protected:

  AWeapon();

  void BeginPlay() override;

public:

  void OnDropped() override;

  bool IsDroppable() const override;

public:

  void StartShooting();

  void StopShooting();

  void Reload();

  UFUNCTION(BlueprintPure)
  bool IsReloading() const;

  UFUNCTION(BlueprintPure)
  bool IsMagazineExtracted() const;

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

  bool IsAmmo() const;

  void OnNoAmmoLeft();

  virtual void InitFromTable();

  const FWeaponRecoilParams & GetRecoil() const;

  bool CanBeShoot() const;

  void ApplyFireModeAmmoLimit();

  void ResetFireModeAmmoLimit();

  UFUNCTION(BlueprintImplementableEvent)
  void SpawnShootProjectile();

  void Fire();

  void ShootProjectile();

  UFUNCTION(BlueprintCallable)
  void PlaySound(EWeaponSound SoundType);

  UFUNCTION(BlueprintImplementableEvent)
  void PlayReloadAnimation();

  UFUNCTION(BlueprintImplementableEvent)
  void PlayShootAnimation();

//
// Replication
//

  void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const override;

  UFUNCTION(Client, Unreliable)
  void Client_PlaySound(EWeaponSound SoundType);

  UFUNCTION(Server, Reliable)
  void Server_Reload();

  UFUNCTION(NetMulticast, Reliable)
  void Multicast_PlayReloadAnimation();

  UFUNCTION(NetMulticast, Unreliable)
  void Multicast_PlayShootAnimation();

  UFUNCTION(Server, Reliable, WithValidation)
  void Server_StartShooting();

  UFUNCTION(Server, Reliable)
  void Server_StopShooting();

  UFUNCTION(Client, Reliable)
  void Client_WeaponShoot(const FWeaponRecoilParams & Recoil);

  UFUNCTION(Server, Reliable)
  void Server_SwitchFireMode();

  UFUNCTION(Client, Unreliable)
  void Client_FireModeChanged(EWeaponFireMode Mode);

  UFUNCTION(Client, Unreliable)
  void Client_MagAmountChanged(int32 MagAmount);

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

  UPROPERTY(EditDefaultsOnly, Category=Projectile)
  TSubclassOf<class ABulletProjectile> ProjectileClass;

  UPROPERTY(BlueprintReadOnly, Replicated, Category=Magazine)
  int32 CurrentMagAmount = 0;

  UPROPERTY(BlueprintReadWrite)
  UMeshComponent * ModelMesh = nullptr;

  UPROPERTY(BlueprintReadOnly, Replicated)
  EWeaponFireMode FireMode = EWeaponFireMode::Single;

  UPROPERTY(Replicated)
  bool bIsShooting = false;

  UPROPERTY(Replicated)
  bool bIsReloading = false;

  UPROPERTY(Replicated)
  uint32 LimitAmmoToShoot = 0u;

protected: // Data table

  TMap<EWeaponSound, FSoundsSet>                 * Sounds         = nullptr;
  TMap<TSoftObjectPtr<UPhysicalMaterial>, int32> * HitboxDamage   = nullptr;
  TArray<EWeaponFireMode>                        * FireModes      = nullptr;
  FWeaponRecoilParams                            * RecoilParams   = nullptr;
  float                                            FireRate       = 0.1f;
  int32                                            MagazineAmount = 0;

protected:

  static const int32 INVALID_AMMO_AMOUNT;

  FTimerHandle m_ShootingTimer;

};
