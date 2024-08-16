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

  UFUNCTION(BlueprintPure)
  AInteractableActor * GetClosestInteractable(float MaxDistanceSquared) const;

  UFUNCTION(BlueprintCallable)
  void TryCreateHint();

  void TryDestroyHint();

protected:

  UPROPERTY(BlueprintReadOnly, Category=Weapon)
  AWeapon * Weapon = nullptr;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Stats)
  int32 MaxHealth = 0;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Stats)
  int32 Health = 0;

  UPROPERTY(EditDefaultsOnly, Category=Interactable)
  float InteractableDiscoverDistanceSquared = 0.0f;

  UPROPERTY(EditDefaultsOnly, Category = Projectile)
  TSubclassOf<class AHint> HintClass;

  TPair<AInteractableActor*, AHint*> HintToInteractable;

};
