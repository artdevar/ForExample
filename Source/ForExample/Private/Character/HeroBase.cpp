#include "Character/HeroBase.h"
#include "Weapon/Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Math/UnrealMathUtility.h"

AHeroBase::AHeroBase()
{
  PrimaryActorTick.bCanEverTick = true;
}

void AHeroBase::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);
}

void AHeroBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  DropWeapon();

  Super::EndPlay(EndPlayReason);
}

void AHeroBase::PickupWeapon(AWeapon * NewWeapon)
{
  if (HasWeapon())
    DropWeapon();

  Weapon = NewWeapon;
  Weapon->SetOwner(this);
  Weapon->OnWeaponShoot.AddDynamic(this, &AHeroBase::OnWeaponShoot);

  const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
  Weapon->AttachToComponent(GetMesh(), AttachmentRules, TEXT("weapon_socket"));

  OnWeaponPickedUp.Broadcast(Weapon);
}

void AHeroBase::DropWeapon()
{
  if (!HasWeapon())
    return;

  const FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepRelative, false);
  Weapon->DetachFromActor(DetachmentRules);

  Weapon->OnWeaponShoot.RemoveDynamic(this, &AHeroBase::OnWeaponShoot);
  Weapon->SetOwner(nullptr);

  OnWeaponDropped.Broadcast(Weapon);

  Weapon->Destroy();
  Weapon = nullptr;
}

bool AHeroBase::HasWeapon() const
{
  return Weapon != nullptr;
}
