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
#include "Net/UnrealNetwork.h"

AHeroBase::AHeroBase()
{
  PrimaryActorTick.bCanEverTick = true;
  bReplicates = true;
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

  Super::Reset();
}

void AHeroBase::BeginPlay()
{
  Super::BeginPlay();

  InitInput();

  GetComponentByClass<URecoilHandler>()->Deactivate();

  bLookForInteractables = IsLocallyControlled();
}

void AHeroBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
}

void AHeroBase::InputActionInteract()
{
  AInteractableActor * NearestInteractable = HintToInteractable ? HintToInteractable->Interactable : nullptr;
  if (NearestInteractable != nullptr)
    Server_PickupInteractable(NearestInteractable);
}

void AHeroBase::InputActionDrop()
{
  if (IsLocallyControlled())
  {
    if (HasWeapon() && Weapon->IsDroppable())
      Server_DropInteractable();
  }
}

void AHeroBase::InputActionAim()
{
  if (IsLocallyControlled())
    Server_Aim();
}

void AHeroBase::InputActionFinishedAim()
{
  if (IsLocallyControlled())
    Server_FinishAim();
}

void AHeroBase::InputActionAttack()
{
  if (IsLocallyControlled())
  {
    if (HasWeapon())
      Weapon->StartShooting();
  }
}

void AHeroBase::InputActionFinishedAttack()
{
  if (IsLocallyControlled())
  {
    if (HasWeapon())
      Weapon->StopShooting();
  }
}

void AHeroBase::InputActionRun()
{
  if (IsLocallyControlled())
    Server_Run();
}

void AHeroBase::InputActionFinishedRun()
{
  if (IsLocallyControlled())
    Server_FinishRun();
}

void AHeroBase::InputActionCrouch()
{
  if (IsLocallyControlled())
    Crouch();
}

void AHeroBase::InputActionFinishedCrouch()
{
  if (IsLocallyControlled())
    UnCrouch();
}

void AHeroBase::InputActionWalk()
{
  if (IsLocallyControlled())
    Server_Walk();
}

void AHeroBase::InputActionFinishedWalk()
{
  if (IsLocallyControlled())
    Server_FinishWalk();
}

void AHeroBase::InputActionReload()
{
  if (IsLocallyControlled())
  {
    if (IsRunning() || !HasWeapon() || IsWeaponReloading())
      return;

    Weapon->Reload();
  }
}

void AHeroBase::InputActionSwitchFireMode()
{
  if (IsLocallyControlled())
  {
    if (HasWeapon())
      Weapon->SwitchFireMode();
  }
}

float AHeroBase::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser)
{
  const float DamageApplied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
  GetPlayerState<AHeroState>()->DecreaseHealth(FMath::RoundToInt32(DamageApplied));
  return DamageApplied;
}

void AHeroBase::OnPlayerStateChanged(APlayerState * NewPlayerState, APlayerState * OldPlayerState)
{
  Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);

  if (HasAuthority() || !NewPlayerState)
    return;

  auto HeroState = Cast<AHeroState>(NewPlayerState);
  OnHealthChanged.Broadcast(HeroState->Health, HeroState->MaxHealth);
}

void AHeroBase::AddControlRotation(const FRotator & NewRotation)
{
  if (IsLocallyControlled())
    GetController()->SetControlRotation(GetControlRotation() + NewRotation);
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

void AHeroBase::OnHealthPointsChanged()
{
  auto HeroState = GetPlayerState<AHeroState>();
  OnHealthChanged.Broadcast(HeroState->Health, HeroState->MaxHealth);
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

//
// Replication
//

void AHeroBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(AHeroBase, Weapon);
  DOREPLIFETIME(AHeroBase, bIsAiming);
}

void AHeroBase::SetWalkingSpeed_Implementation(float Speed)
{
  GetCharacterMovement()->MaxWalkSpeed = Speed;
}

void AHeroBase::SetRunning_Implementation(bool bIsRunning)
{
  if (bIsRunning)
  {
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = !bUseControllerRotationYaw;
    SetWalkingSpeed(RunSpeed);
  }
  else
  {
    bUseControllerRotationYaw = HasWeapon() && bApplyControllerRotationYawWithWeapon;
    GetCharacterMovement()->bOrientRotationToMovement = !bUseControllerRotationYaw;
    SetWalkingSpeed(WalkSpeed);
  }
}

void AHeroBase::OnRep_WeaponChanged(AWeapon * PrevWeapon)
{
  const bool WeaponChanged = PrevWeapon != nullptr;
  const bool WeaponDropped = Weapon     == nullptr;

  if (WeaponChanged)
  {
    if (PrevWeapon->IsReloading())
      StopAnimMontage();

    PrevWeapon->StopShooting();

    const FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, false);
    PrevWeapon->DetachFromActor(DetachmentRules);

    auto WeaponRootComponent = Cast<UPrimitiveComponent>(PrevWeapon->GetRootComponent());
    WeaponRootComponent->SetSimulatePhysics(true);
    WeaponRootComponent->AddImpulse(GetActorForwardVector() * DropImpulse, NAME_None, true);

    PrevWeapon->OnWeaponShoot.RemoveDynamic(this, &AHeroBase::OnWeaponShoot);
    PrevWeapon->OnDropped();
    PrevWeapon->SetPickupable(false);
    PrevWeapon->SetOwner(nullptr);

    OnWeaponDropped.Broadcast(PrevWeapon);

    FTimerHandle Timer;
    GetWorld()->GetTimerManager().SetTimer(Timer, [WeaponPtr = PrevWeapon]()
    {
      if (::IsValid(WeaponPtr))
        WeaponPtr->SetPickupable(true);
    }, 1.0f, false);

    GetComponentByClass<URecoilHandler>()->Deactivate();
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = !bUseControllerRotationYaw;
  }

  if (!WeaponDropped)
  {
    Cast<UPrimitiveComponent>(Weapon->GetRootComponent())->SetSimulatePhysics(false);
    Weapon->SetOwner(this);
    Weapon->OnPickedUp();
    Weapon->OnWeaponShoot.AddDynamic(this, &AHeroBase::OnWeaponShoot);

    GetComponentByClass<URecoilHandler>()->Activate(true);

    const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
    Weapon->AttachToComponent(GetMesh(), AttachmentRules, TEXT("weapon_socket"));

    OnWeaponPickedUp.Broadcast(Weapon);
    bUseControllerRotationYaw = bApplyControllerRotationYawWithWeapon;
    GetCharacterMovement()->bOrientRotationToMovement = !bUseControllerRotationYaw;
  }
}

void AHeroBase::Server_PickupInteractable_Implementation(AInteractableActor * Interactable)
{
  switch (Interactable->GetType())
  {
    case EInteractableType::Weapon:
    {
      auto PrevWeapon = Weapon;
      Weapon = Cast<AWeapon>(Interactable);
      OnRep_WeaponChanged(PrevWeapon);
      break;
    }
  }
}

bool AHeroBase::Server_PickupInteractable_Validate(AInteractableActor * Interactable)
{
  return GetClosestInteractable() == Interactable;
}

void AHeroBase::Server_DropInteractable_Implementation()
{
  auto PrevWeapon = Weapon;
  Weapon = nullptr;
  OnRep_WeaponChanged(PrevWeapon);
}

bool AHeroBase::Server_DropInteractable_Validate()
{
  return HasWeapon() && Weapon->IsDroppable();
}

void AHeroBase::OnRep_AimChanged()
{
  if (bIsAiming)
  {
    bUseControllerRotationYaw = HasWeapon();
    GetCharacterMovement()->bOrientRotationToMovement = !bUseControllerRotationYaw;
    SetWalkingSpeed(SlowWalkSpeed);
  }
  else
  {
    bUseControllerRotationYaw = HasWeapon() && bApplyControllerRotationYawWithWeapon;
    GetCharacterMovement()->bOrientRotationToMovement = !bUseControllerRotationYaw;
    SetWalkingSpeed(WalkSpeed);
  }
}

void AHeroBase::Server_Aim_Implementation()
{
  if (!bIsAiming)
  {
    bIsAiming = true;
    OnRep_AimChanged();
  }
}

void AHeroBase::Server_FinishAim_Implementation()
{
  if (bIsAiming)
  {
    bIsAiming = false;
    OnRep_AimChanged();
  }
}

void AHeroBase::Server_Run_Implementation()
{
  if (IsWeaponAiming() || IsWeaponReloading() || bIsCrouched)
    return;

  SetRunning(true);
}

void AHeroBase::Server_FinishRun_Implementation()
{
  if (!IsRunning())
    return;

  SetRunning(false);
}

void AHeroBase::Server_Walk_Implementation()
{
  if (IsWeaponAiming() || bIsCrouched)
    return;

  SetWalkingSpeed(SlowWalkSpeed);
}

void AHeroBase::Server_FinishWalk_Implementation()
{
  if (IsWeaponAiming() || bIsCrouched)
    return;

  SetWalkingSpeed(WalkSpeed);
}