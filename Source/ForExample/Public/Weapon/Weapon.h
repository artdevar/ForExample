#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponSound : uint8
{
  Shoot  UMETA(DisplayName="Shoot"),
  Reload UMETA(DisplayName="Reload"),
  NoAmmo UMETA(DisplayName="NoAmmo")
};

UCLASS()
class FOREXAMPLE_API AWeapon : public AActor
{
  GENERATED_BODY()

public:

  AWeapon();

  void BeginPlay() override;

public:

  UFUNCTION(BlueprintCallable)
  virtual void Shoot();

  UFUNCTION(BlueprintCallable)
  virtual void Reload(); 

protected:

  virtual bool CanBeShoot() const;

  UFUNCTION(BlueprintImplementableEvent)
  void PlaySound(EWeaponSound Action);

protected:

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Magazine")
  int32 MagazineAmount = 0;

  UPROPERTY(BlueprintReadWrite, Category="Magazine")
  int32 CurrentMagazineAmount = 0;

};
