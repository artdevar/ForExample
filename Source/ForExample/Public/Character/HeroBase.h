#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HeroBase.generated.h"

class AWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponPickedUpSignature, AWeapon*, Weapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponDroppedSignature, AWeapon*, Weapon);

UCLASS()
class FOREXAMPLE_API AHeroBase : public ACharacter
{
  GENERATED_BODY()

public:

  AHeroBase();

  void Tick(float DeltaSeconds) override;
  
  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public: 

  UFUNCTION(BlueprintCallable, Category=Weapon)
  void PickupWeapon(AWeapon * NewWeapon);

  UFUNCTION(BlueprintCallable, Category=Weapon)
  void DropWeapon();

  UFUNCTION(BlueprintPure, Category=Weapon)
  bool HasWeapon() const;

  UFUNCTION(BlueprintImplementableEvent, BlueprintPure, Category=Weapon)
  bool IsWeaponAiming() const;

public: 

  UPROPERTY(BlueprintAssignable, Category=Weapon)
  FWeaponPickedUpSignature OnWeaponPickedUp;

  UPROPERTY(BlueprintAssignable, Category=Weapon)
  FWeaponDroppedSignature OnWeaponDropped;

protected:

  UFUNCTION(BlueprintImplementableEvent, Category=Weapon)
  void OnWeaponShoot(FVector2D Recoil);

protected:

  UPROPERTY(BlueprintReadOnly, Category=Weapon)
  AWeapon * Weapon = nullptr;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Stats)
  int32 Health = 0;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Stats)
  int32 Armor = 0;

};
