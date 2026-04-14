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
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

AHeroBase::AHeroBase()
{
  PrimaryActorTick.bCanEverTick = true;
  bReplicates                   = true;
}

void AHeroBase::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);
}

void AHeroBase::Reset()
{
  if (GetWorldTimerManager().IsTimerActive(LookForInteractableTimer))
  {
    GetWorldTimerManager().ClearTimer(LookForInteractableTimer);
    DestroyHint();
  }

  Super::Reset();
}

void AHeroBase::BeginPlay()
{
  Super::BeginPlay();

  InitInput();

  GetComponentByClass<URecoilHandler>()->Deactivate();

  if (IsLocallyControlled())
  {
    CreateHint();
    GetWorldTimerManager().SetTimer(LookForInteractableTimer, this, &AHeroBase::UpdateHint, 0.05f, true);
  }
}

void AHeroBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
}

void AHeroBase::InputActionInteract()
{
  if (HintToInteractable->Interactable.IsValid())
  {
    Server_PickupInteractable(HintToInteractable->Interactable.Get());
    UpdateHint();
  }
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
  if (IsLocallyControlled() && !IsRunning())
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
    if (IsRunning() || !HasWeapon() || IsWeaponReloading() || IsWeaponAmmoFull())
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

float AHeroBase::TakeDamage(float DamageAmount, struct FDamageEvent const &DamageEvent, class AController *EventInstigator, AActor *DamageCauser)
{
  const float DamageApplied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
  GetPlayerState<AHeroState>()->DecreaseHealth(FMath::RoundToInt32(DamageApplied));
  return DamageApplied;
}

void AHeroBase::OnPlayerStateChanged(APlayerState *NewPlayerState, APlayerState *OldPlayerState)
{
  Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);

  if (HasAuthority() || !NewPlayerState)
    return;

  auto HeroState = Cast<AHeroState>(NewPlayerState);
  OnHealthChanged.Broadcast(HeroState->Health, HeroState->MaxHealth);
}

void AHeroBase::AddControlRotation(const FRotator &NewRotation)
{
  if (IsLocallyControlled())
    GetController()->SetControlRotation(GetControlRotation() + NewRotation);
}

bool AHeroBase::HasWeapon() const
{
  return !Weapon.IsExplicitlyNull() && Weapon->IsPickedUp();
}

bool AHeroBase::IsWeaponAiming() const
{
  return HasWeapon() && bIsAiming;
}

bool AHeroBase::IsWeaponReloading() const
{
  return HasWeapon() && Weapon->IsReloading();
}

bool AHeroBase::IsWeaponAmmoFull() const
{
  return HasWeapon() && Weapon->HasFullAmmo();
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

bool AHeroBase::IsWalking() const
{
  return FMath::IsNearlyEqual(GetCharacterMovement()->MaxWalkSpeed, WalkSpeed);
}

void AHeroBase::OnWeaponShoot(FWeaponRecoilParams RecoilParams)
{
  auto RecoilHandler = GetComponentByClass<URecoilHandler>();
  RecoilHandler->Add(RecoilParams);
  PlayWeaponShootAnimation();
}

void AHeroBase::SetUseControllerRotationYaw(bool bUse)
{
  bUseControllerRotationYaw                         = bUse;
  GetCharacterMovement()->bOrientRotationToMovement = !bUseControllerRotationYaw;
}

AInteractableActor *AHeroBase::GetClosestInteractable() const
{
  const FVector HeroLocation            = GetActorLocation();
  const float   DiscoverDistanceSquared = InteractableDiscoverDistance * InteractableDiscoverDistance;

  FCollisionShape     Sphere          = FCollisionShape::MakeSphere(InteractableDiscoverDistance);
  AInteractableActor *Interactable    = nullptr;
  float               ClosestDistance = FLT_MAX;

  TArray<FOverlapResult> OverlapResults;
  FCollisionQueryParams  QueryParams;
  QueryParams.AddIgnoredActor(this);

  GetWorld()->OverlapMultiByChannel(OverlapResults, GetActorLocation(), FQuat::Identity, ECC_Visibility, Sphere, QueryParams);

  for (const FOverlapResult &Result : OverlapResults)
  {
    AActor *Actor = Result.GetActor();

    const float DistanceDeltaSquared = (Actor->GetActorLocation() - HeroLocation).SquaredLength();
    if (DistanceDeltaSquared > DiscoverDistanceSquared)
      continue;

    if (DistanceDeltaSquared < ClosestDistance)
    {
      if (AInteractableActor *InteractableActor = Cast<AInteractableActor>(Actor); InteractableActor && InteractableActor->IsPickupable())
      {
        Interactable    = InteractableActor;
        ClosestDistance = DistanceDeltaSquared;
      }
    }
  }

  return Interactable;
}

void AHeroBase::CreateHint()
{
  ensure(HintToInteractable == nullptr);

  FActorSpawnParameters HintSpawnParams;
  HintSpawnParams.Owner                          = this;
  HintSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

  HintToInteractable                              = GetWorld()->SpawnActor<AHint>(HintClass, FTransform(), HintSpawnParams);
  HintToInteractable->IsAttachedToInteractable    = true;
  HintToInteractable->DistanceDiscoverableSquared = InteractableDiscoverDistance * InteractableDiscoverDistance;
}

void AHeroBase::UpdateHint()
{
  AInteractableActor *ClosestInteractable = GetClosestInteractable();
  HintToInteractable->ChangeInteractable(ClosestInteractable, ClosestInteractable ? EHintAction::Pickup : EHintAction::None);
}

void AHeroBase::DestroyHint()
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

void AHeroBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
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
    SetUseControllerRotationYaw(false);
    SetWalkingSpeed(RunSpeed);
  }
  else
  {
    SetUseControllerRotationYaw(HasWeapon() && bApplyControllerRotationYawWithWeapon);
    SetWalkingSpeed(WalkSpeed);
  }
}

void AHeroBase::OnRep_WeaponChanged(const TWeakObjectPtr<AWeapon> &PrevWeapon)
{
  const bool WeaponChanged = !PrevWeapon.IsExplicitlyNull();
  const bool WeaponDropped = Weapon.IsExplicitlyNull();

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

    OnWeaponDropped.Broadcast(PrevWeapon.Get());

    FTimerHandle Timer;
    GetWorld()->GetTimerManager().SetTimer(
        Timer,
        [WeaponPtr = PrevWeapon]() {
          if (WeaponPtr.IsValid())
            WeaponPtr->SetPickupable(true);
        },
        1.0f, false);

    GetComponentByClass<URecoilHandler>()->Deactivate();
    SetUseControllerRotationYaw(false);
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

    OnWeaponPickedUp.Broadcast(Weapon.Get());
    SetUseControllerRotationYaw(bApplyControllerRotationYawWithWeapon);
  }
}

void AHeroBase::Server_PickupInteractable_Implementation(AInteractableActor *Interactable)
{
  switch (Interactable->GetType())
  {
  case EInteractableType::Weapon: {
    auto PrevWeapon = Weapon;
    Weapon          = Cast<AWeapon>(Interactable);
    OnRep_WeaponChanged(PrevWeapon.Get());
    break;
  }
  }
}

bool AHeroBase::Server_PickupInteractable_Validate(AInteractableActor *Interactable)
{
  return GetClosestInteractable() == Interactable;
}

void AHeroBase::Server_DropInteractable_Implementation()
{
  auto PrevWeapon = Weapon;
  Weapon          = nullptr;
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
    SetUseControllerRotationYaw(HasWeapon());
    SetWalkingSpeed(SlowWalkSpeed);
  }
  else
  {
    SetUseControllerRotationYaw(HasWeapon() && bApplyControllerRotationYawWithWeapon);
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
  const bool IsMoving = !GetCharacterMovement()->GetCurrentAcceleration().IsNearlyZero();

  if (!IsMoving || IsWeaponAiming() || IsWeaponReloading() || bIsCrouched)
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
