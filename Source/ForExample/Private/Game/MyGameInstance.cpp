#include "Game/MyGameInstance.h"

USoundBase * UMyGameInstance::GetRandomWeaponSound(TSubclassOf<class AWeapon> WeaponClass, EWeaponSound SoundType) const
{
  const FWeaponSoundsSet * WeaponSoundsSet = WeaponSounds.Find(WeaponClass);
  if (!WeaponSoundsSet)
    return nullptr;

  const FSoundsSet * SoundsSet = WeaponSoundsSet->WeaponSounds.Find(SoundType);
  if (!SoundsSet)
    return nullptr;

  if (SoundsSet->Sounds.IsEmpty())
    return nullptr;

  return SoundsSet->Sounds[FMath::RandRange(0, SoundsSet->Sounds.Num() - 1)].Get();
}

USoundBase * UMyGameInstance::GetRandomObstacleHitSound() const
{
  if (ObstacleHitSounds.Sounds.IsEmpty())
    return nullptr;

  return ObstacleHitSounds.Sounds[FMath::RandRange(0, ObstacleHitSounds.Sounds.Num() - 1)].Get();
}