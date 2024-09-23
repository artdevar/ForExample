#include "Character/HeroBase.h"
#include "Character/HeroState.h"
#include "Weapon/Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/Hint.h"
#include "Weapon/RecoilHandler.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

AHeroBase::AHeroBase()
{
  PrimaryActorTick.bCanEverTick = true;
  bLookForInteractables = true;
}

void AHeroBase::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);

  if (bLookForInteractables)
    TryCreateHint();
}

void AHeroBase::Reset()
{
  bLookForInteractables = false;
  TryDestroyHint();
  DropWeapon();

  Super::Reset();
}

void AHeroBase::BeginPlay()
{
  Super::BeginPlay();

  InitInput();
}

void AHeroBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
}

void AHeroBase::InputActionUse()
{
  AInteractableActor * NearestInteractable = HintToInteractable ? HintToInteractable->Interactable : nullptr;
  if (NearestInteractable == nullptr)
    return;

  switch (NearestInteractable->GetType())
  {
    case EInteractableType::Weapon:
    {
      if (HasWeapon())
        DropWeapon();

      Weapon = Cast<AWeapon>(NearestInteractable);
      Weapon->GetComponentByClass<USphereComponent>()->SetSimulatePhysics(false);
      Weapon->SetOwner(this);
      Weapon->OnPickedUp();
      Weapon->OnWeaponShoot.AddDynamic(this, &AHeroBase::OnWeaponShoot);

      GetComponentByClass<URecoilHandler>()->Activate(true);

      const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
      Weapon->AttachToComponent(GetMesh(), AttachmentRules, TEXT("weapon_socket"));

      OnWeaponPickedUp.Broadcast(Weapon);

      bUseControllerRotationYaw = bApplyControllerRotationYawWithWeapon;

      break;
    }

    default:
      break;
  }

  TryCreateHint();
}

void AHeroBase::InputActionDrop()
{
  if (HasWeapon() && Weapon->IsDroppable())
    DropWeapon();
}

void AHeroBase::InputActionAim()
{
  bIsAiming = true;
  bUseControllerRotationYaw = HasWeapon();
  SetWalkingSpeed(SlowWalkSpeed);
}

void AHeroBase::InputActionFinishedAim()
{
  bIsAiming = false;
  bUseControllerRotationYaw = HasWeapon() && bApplyControllerRotationYawWithWeapon;
  SetWalkingSpeed(WalkSpeed);
}

void AHeroBase::InputActionAttack()
{
  if (HasWeapon())
    Weapon->StartShooting();
}

void AHeroBase::InputActionFinishedAttack()
{
  if (HasWeapon())
    Weapon->StopShooting();
}

void AHeroBase::InputActionRun()
{
  if (IsWeaponAiming() || IsWeaponReloading() || bIsCrouched)
    return;

  bUseControllerRotationYaw = false;
  SetWalkingSpeed(RunSpeed);
}

void AHeroBase::InputActionFinishedRun()
{
  if (IsWeaponAiming() || IsWeaponReloading() || bIsCrouched)
    return;

  bUseControllerRotationYaw = HasWeapon() && bApplyControllerRotationYawWithWeapon;
  SetWalkingSpeed(WalkSpeed);
}

void AHeroBase::InputActionCrouch()
{
  Crouch();
}

void AHeroBase::InputActionFinishedCrouch()
{
  UnCrouch();
}

void AHeroBase::InputActionWalk()
{
  if (IsWeaponAiming() || bIsCrouched)
    return;

  SetWalkingSpeed(SlowWalkSpeed);
}

void AHeroBase::InputActionFinishedWalk()
{
  SetWalkingSpeed(WalkSpeed);
}

void AHeroBase::InputActionReload()
{
  if (!HasWeapon() || IsWeaponReloading() || IsRunning())
    return;

  Weapon->Reload();
  PlayWeaponReloadAnimation();
}

void AHeroBase::InputActionSwitchFireMode()
{
  if (HasWeapon())
    Weapon->SwitchFireMode();
}

float AHeroBase::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser)
{
  const float DamageApplied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
  GetPlayerState<AHeroState>()->DecreaseHealth(FMath::RoundToInt32(DamageApplied));
  return DamageApplied;
}

void AHeroBase::AddControlRotation(const FRotator & NewRotation)
{
  GetController()->SetControlRotation(GetControlRotation() + NewRotation);
}

void AHeroBase::DropWeapon()
{
  if (Weapon->IsReloading())
    StopAnimMontage();

  Weapon->StopShooting();

  const FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, false);
  Weapon->DetachFromActor(DetachmentRules);

  auto WeaponRootComponent = Cast<UPrimitiveComponent>(Weapon->GetRootComponent());
  WeaponRootComponent->SetSimulatePhysics(true);
  WeaponRootComponent->AddImpulse(GetActorForwardVector() * DropImpulse, NAME_None, true);

  Weapon->OnWeaponShoot.RemoveDynamic(this, &AHeroBase::OnWeaponShoot);
  Weapon->OnDropped();
  Weapon->SetPickupable(false);
  Weapon->SetOwner(nullptr);

  OnWeaponDropped.Broadcast(Weapon);

  FTimerHandle Timer;
  GetWorld()->GetTimerManager().SetTimer(Timer, [WeaponPtr = Weapon]()
  {
    if (::IsValid(WeaponPtr))
      WeaponPtr->SetPickupable(true);
  }, 1.0f, false);

  GetComponentByClass<URecoilHandler>()->Deactivate();

  Weapon = nullptr;
  bUseControllerRotationYaw = false;
}

bool AHeroBase::HasWeapon() const
{
  return Weapon && Weapon->IsPickedUp();
}

bool AHeroBase::IsWeaponAiming() const
{
  return HasWeapon() && bIsAiming;
}

bool AHeroBase::IsWeaponReloading() const
{
  return HasWeapon() && Weapon->IsReloading();
}

void AHeroBase::OnNoHealthLeft()
{

}

bool AHeroBase::CanJumpInternal_Implementation() const
{
  return Super::CanJumpInternal_Implementation() && !IsWeaponAiming();
}

bool AHeroBase::CanCrouch() const
{
  // No animations for crouching without a weapon
  return Super::CanCrouch() && HasWeapon();
}

void AHeroBase::SetWalkingSpeed(float Speed)
{
  GetCharacterMovement()->MaxWalkSpeed = Speed;
}

bool AHeroBase::IsRunning() const
{
  return FMath::IsNearlyEqual(GetCharacterMovement()->MaxWalkSpeed, RunSpeed);
}

void AHeroBase::OnWeaponShoot(FWeaponRecoilParams RecoilParams)
{
  auto RecoilHandler = GetComponentByClass<URecoilHandler>();
  RecoilHandler->Add(RecoilParams);
  PlayWeaponShootAnimation();
}

AInteractableActor * AHeroBase::GetClosestInteractable() const
{
  const FVector HeroLocation            = GetActorLocation();
  const float   DiscoverDistanceSquared = InteractableDiscoverDistance * InteractableDiscoverDistance;

  FCollisionShape      Sphere          = FCollisionShape::MakeSphere(InteractableDiscoverDistance);
  AInteractableActor * Interactable    = nullptr;
  float                ClosestDistance = FLT_MAX;

  TArray<FOverlapResult> OverlapResults;
  FCollisionQueryParams QueryParams;
  QueryParams.AddIgnoredActor(this);

  GetWorld()->OverlapMultiByChannel(OverlapResults, GetActorLocation(), FQuat::Identity, ECC_Visibility, Sphere, QueryParams);

  for (const FOverlapResult & Result : OverlapResults)
  {
    AActor * Actor = Result.GetActor();

    const float DistanceDeltaSquared = (Actor->GetActorLocation() - HeroLocation).SquaredLength();
    if (DistanceDeltaSquared > DiscoverDistanceSquared)
      continue;

    if (DistanceDeltaSquared < ClosestDistance)
    {
      if (AInteractableActor * InteractableActor = Cast<AInteractableActor>(Actor);
          InteractableActor && InteractableActor->IsPickupable())
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
  AInteractableActor * ClosestInteractable = GetClosestInteractable();

  if (!ClosestInteractable)
  {
    TryDestroyHint();
    return;
  }

  if (HintToInteractable)
  {
    HintToInteractable->Interactable = ClosestInteractable;
    return;
  }

  FActorSpawnParameters HintSpawnParams;
  HintSpawnParams.Owner = this;
  HintSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

  HintToInteractable = GetWorld()->SpawnActor<AHint>(HintClass, ClosestInteractable->GetTransform(), HintSpawnParams);
  HintToInteractable->HintAction                  = EHintAction::Pickup;
  HintToInteractable->Interactable                = ClosestInteractable;
  HintToInteractable->IsAttachedToInteractable    = true;
  HintToInteractable->NeedFacing                  = true;
  HintToInteractable->DistanceDiscoverableSquared = InteractableDiscoverDistance * InteractableDiscoverDistance;
}

void AHeroBase::TryDestroyHint()
{
  if (HintToInteractable)
  {
    HintToInteractable->Destroy();
    HintToInteractable = nullptr;
  }
}