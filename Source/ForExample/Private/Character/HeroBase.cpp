#include "Character/HeroBase.h"
#include "Weapon/Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/Hint.h"
#include "Weapon/RecoilHandler.h"
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

void AHeroBase::AddControlRotation(const FRotator & NewRotation)
{
  GetController()->SetControlRotation(GetControlRotation() + NewRotation);
}

void AHeroBase::UseButtonPressed()
{
  AInteractableActor * NearestInteractable = HintToInteractable.Key;
  if (NearestInteractable == nullptr)
    return;

  switch (NearestInteractable->GetType())
  {
    case EInteractableType::Weapon:
    {
      if (HasWeapon())
        DropWeapon();

      Weapon = Cast<AWeapon>(NearestInteractable);
      Weapon->SetOwner(this);
      Weapon->OnPickedUp();
      Weapon->OnWeaponShoot.AddDynamic(this, &AHeroBase::OnWeaponShoot);

      reinterpret_cast<URecoilHandler *>(GetComponentByClass(URecoilHandler::StaticClass()))->Activate(true);

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

  reinterpret_cast<URecoilHandler *>(GetComponentByClass(URecoilHandler::StaticClass()))->Deactivate();

  Weapon->Destroy();
  Weapon = nullptr;
}

bool AHeroBase::HasWeapon() const
{
  return Weapon != nullptr;
}

bool AHeroBase::IsValid() const
{
  return ::IsValid(this);
}

bool AHeroBase::CanJumpInternal_Implementation() const
{
  return Super::CanJumpInternal_Implementation() && !IsWeaponAiming();
}

void AHeroBase::OnWeaponShoot(FWeaponRecoilParams RecoilParams)
{
  auto RecoilHandler = reinterpret_cast<URecoilHandler *>(GetComponentByClass(URecoilHandler::StaticClass()));
  RecoilHandler->Add(RecoilParams);
  PlayWeaponShootAnimation();
}

AInteractableActor * AHeroBase::GetClosestInteractable(float MaxDistanceSquared) const
{
  const FVector HeroLocation = GetActorLocation();

  AInteractableActor * Interactable = nullptr;
  float ClosestDistance = FLT_MAX;

  TArray<AActor *> AllInteractables;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AInteractableActor::StaticClass(), AllInteractables);

  for (AActor * Actor : AllInteractables)
  {
    const float DistanceDeltaSquared = (Actor->GetActorLocation() - HeroLocation).SquaredLength();

    if (DistanceDeltaSquared > MaxDistanceSquared)
      continue;

    if (DistanceDeltaSquared < ClosestDistance)
    {
      if (AInteractableActor * InteractableActor = reinterpret_cast<AInteractableActor *>(Actor);
         !InteractableActor->IsPickedUp())
      {
        Interactable    = InteractableActor;
        ClosestDistance = DistanceDeltaSquared;
      }
    }
  }

  return Interactable;
}

void AHeroBase::TryCreateHint()
{
  AInteractableActor * ClosestInteractable = GetClosestInteractable(InteractableDiscoverDistanceSquared);
  if (ClosestInteractable == HintToInteractable.Key)
    return;

  TryDestroyHint();

  if (ClosestInteractable == nullptr)
    return;

  FActorSpawnParameters HintSpawnParams;
  HintSpawnParams.Owner = this;

  AHint * Hint = GetWorld()->SpawnActor<AHint>(HintClass, ClosestInteractable->GetTransform(), HintSpawnParams);
  Hint->HintAction                  = EHintAction::Pickup;
  Hint->Interactable                = ClosestInteractable;
  Hint->IsAttachedToInteractable    = true;
  Hint->NeedFacing                  = true;
  Hint->DistanceDiscoverableSquared = InteractableDiscoverDistanceSquared;

  HintToInteractable.Key   = ClosestInteractable;
  HintToInteractable.Value = Hint;
}

void AHeroBase::TryDestroyHint()
{
  if (HintToInteractable.Value == nullptr)
    return;

  HintToInteractable.Value->Destroy();
  HintToInteractable.Value = nullptr;
  HintToInteractable.Key   = nullptr;
}