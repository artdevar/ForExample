#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Weapon/Weapon.h"
#include "MyGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FSoundsSet
{
  GENERATED_BODY()

public:

  UPROPERTY(EditDefaultsOnly)
  TArray<TObjectPtr<USoundBase>> Sounds;
};

USTRUCT(BlueprintType)
struct FWeaponSoundsSet
{
  GENERATED_BODY()

public:

  UPROPERTY(EditDefaultsOnly)
  TMap<EWeaponSound, FSoundsSet> WeaponSounds;
};

UCLASS()
class FOREXAMPLE_API UMyGameInstance : public UGameInstance
{
  GENERATED_BODY()

public:

  UFUNCTION(BlueprintCallable, BlueprintPure)
  USoundBase * GetRandomWeaponSound(TSubclassOf<class AWeapon> WeaponClass, EWeaponSound SoundType) const;

  UFUNCTION(BlueprintCallable, BlueprintPure)
  USoundBase * GetRandomObstacleHitSound() const;

public:

  UPROPERTY(EditDefaultsOnly, Category=Weapons)
  TMap<TSubclassOf<class AWeapon>, FWeaponSoundsSet> WeaponSounds;

  UPROPERTY(EditDefaultsOnly, Category=Weapons)
  FSoundsSet ObstacleHitSounds;

};
