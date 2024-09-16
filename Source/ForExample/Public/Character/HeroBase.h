#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Shared/Weapons.h"
#include "HeroBase.generated.h"

class AWeapon;
class AHint;
class AInteractableActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponPickedUpSignature, AWeapon*, Weapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponDroppedSignature, AWeapon*, Weapon);

UCLASS()
class FOREXAMPLE_API AHeroBase : public ACharacter
{
  GENERATED_BODY()

public:

  AHeroBase();

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) override;

  void AddControlRotation(const FRotator & NewRotation);

  UFUNCTION(BlueprintCallable)
  void UseButtonPressed();

  UFUNCTION(BlueprintCallable, Category=Weapon)
  void DropWeapon();

  UFUNCTION(BlueprintPure, Category=Weapon)
  bool HasWeapon() const;

  UFUNCTION(BlueprintImplementableEvent, BlueprintPure, Category=Weapon)
  bool IsWeaponAiming() const;

  bool IsValid() const;

  void OnNoHealthLeft();

  UPROPERTY(BlueprintAssignable, Category=Weapon)
  FWeaponPickedUpSignature OnWeaponPickedUp;

  UPROPERTY(BlueprintAssignable, Category=Weapon)
  FWeaponDroppedSignature OnWeaponDropped;

protected:

  bool CanJumpInternal_Implementation() const override;

  UFUNCTION(BlueprintCallable)
  void OnWeaponShoot(FWeaponRecoilParams RecoilParams);

  UFUNCTION(BlueprintImplementableEvent, Category=Weapon)
  void PlayWeaponShootAnimation();

  AInteractableActor * GetClosestInteractable() const;

  UFUNCTION(BlueprintCallable)
  void TryCreateHint();

  void TryDestroyHint();

protected:

  UPROPERTY(EditDefaultsOnly, Category=Interactable)
  float InteractableDiscoverDistance = 0.0f;

  UPROPERTY(EditDefaultsOnly, Category=Projectile)
  TSubclassOf<class AHint> HintClass;

  UPROPERTY(EditDefaultsOnly, Category=Weapon)
  bool ApplyControllerRotationYawWithWeapon = false;

  UPROPERTY(BlueprintReadOnly, Category=Weapon)
  AWeapon * Weapon = nullptr;

  AHint * HintToInteractable = nullptr;

};
