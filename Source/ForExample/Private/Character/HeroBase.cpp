#include "Character/HeroBase.h"
#include "Weapon/Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/Hint.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

AHeroBase::AHeroBase()
{
  PrimaryActorTick.bCanEverTick = true;
}

void AHeroBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  DropWeapon();

  Super::EndPlay(EndPlayReason);
}

void AHeroBase::UseButtonPressed()
{
  APickupableActor * NearestPickupable = HintToPickupable.Key;
  if (NearestPickupable == nullptr)
    return;

  switch (NearestPickupable->GetType())
  {
    case EPickupableType::Weapon:
    {
      if (HasWeapon())
        DropWeapon();

      Weapon = Cast<AWeapon>(NearestPickupable);
      Weapon->SetOwner(this);
      Weapon->OnPickedUp();
      Weapon->OnWeaponShoot.AddDynamic(this, &AHeroBase::OnWeaponShoot);

      const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
      Weapon->AttachToComponent(GetMesh(), AttachmentRules, TEXT("weapon_socket"));

      OnWeaponPickedUp.Broadcast(Weapon);

      break;
    }

    default: 
      break;
  }

  TryDestroyHint();
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

APickupableActor * AHeroBase::GetClosestPickupable(float MaxDistanceSquared) const
{
  const FVector HeroLocation = GetActorLocation();

  APickupableActor * Pickupable = nullptr;
  float ClosestDistance = FLT_MAX;

  TArray<AActor *> AllPickupables;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), APickupableActor::StaticClass(), AllPickupables);

  for (AActor * Actor : AllPickupables)
  {
    const float DistanceDeltaSquared = (Actor->GetActorLocation() - HeroLocation).SquaredLength();

    if (DistanceDeltaSquared > MaxDistanceSquared)
      continue;

    if (DistanceDeltaSquared < ClosestDistance)
    {
      if (APickupableActor * PickupableActor = reinterpret_cast<APickupableActor *>(Actor);
         !PickupableActor->IsPickedUp())
      {
        Pickupable      = PickupableActor;
        ClosestDistance = DistanceDeltaSquared;
      }
    }
  }

  return Pickupable;
}

void AHeroBase::TryCreateHint()
{
  APickupableActor * ClosestPickupable = GetClosestPickupable(PickupableDiscoverDistanceSquared);
  if (ClosestPickupable == HintToPickupable.Key)
    return;

  TryDestroyHint();

  if (ClosestPickupable == nullptr)
    return;

  FActorSpawnParameters HintSpawnParams;
  HintSpawnParams.Owner = this;

  AHint * Hint = GetWorld()->SpawnActor<AHint>(HintClass, ClosestPickupable->GetTransform(), HintSpawnParams);
  Hint->HintAction                  = EHintAction::Pickup;
  Hint->Pickupable                  = ClosestPickupable;
  Hint->IsAttachedToPickupable      = true;
  Hint->NeedFacing                  = true;
  Hint->DistanceDiscoverableSquared = PickupableDiscoverDistanceSquared;

  HintToPickupable.Key   = ClosestPickupable;
  HintToPickupable.Value = Hint;
}

void AHeroBase::TryDestroyHint()
{
  if (HintToPickupable.Value == nullptr)
    return;

  HintToPickupable.Value->Destroy();
  HintToPickupable.Value = nullptr;
  HintToPickupable.Key   = nullptr;
}