#include "Weapon/Weapon.h"

AWeapon::AWeapon()
{
  PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::BeginPlay()
{
  Reload();
}

void AWeapon::Shoot()
{
  if (!CanBeShoot())
    return;

  PlaySound(EWeaponSound::Shoot);
  --CurrentMagazineAmount;
}

void AWeapon::Reload()
{
  CurrentMagazineAmount = MagazineAmount;
}

bool AWeapon::CanBeShoot() const
{
  return CurrentMagazineAmount > 0;
}

