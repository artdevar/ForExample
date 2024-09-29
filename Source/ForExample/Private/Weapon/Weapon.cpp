#include "Weapon/Weapon.h"
#include "Weapon/BulletProjectile.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Character/HeroBase.h"
#include "Components/MeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Data/WeaponData.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

const int32 AWeapon::INVALID_AMMO_AMOUNT = -1;

AWeapon::AWeapon()
{
  PrimaryActorTick.bCanEverTick = true;
  bReplicates = true;
}

void AWeapon::BeginPlay()
{
  Super::BeginPlay();

  InitFromTable();

  FireMode = FireModes->Contains(EWeaponFireMode::Automatic) ? EWeaponFireMode::Automatic :
             FireModes->Contains(EWeaponFireMode::Burst)     ? EWeaponFireMode::Burst     : EWeaponFireMode::Single;

  CurrentMagAmount = FMath::RandRange(MagazineAmount / 3, MagazineAmount);
  OnWeaponAmmoChanged.Broadcast(CurrentMagAmount);
}

void AWeapon::OnDropped()
{
  bIsReloading = false;

  Super::OnDropped();
}

bool AWeapon::IsDroppable() const
{
  return Super::IsDroppable() && !IsReloading();
}

void AWeapon::StartShooting()
{
  if (IsReloading())
    return;

  if (!IsAmmo())
  {
    OnNoAmmoLeft();
    return;
  }

  Server_StartShooting();
}

void AWeapon::StopShooting()
{
  Server_StopShooting();
}

void AWeapon::OnNoAmmoLeft()
{
  PlaySound(EWeaponSound::NoAmmo);
}

void AWeapon::InitFromTable()
{
  FWeaponData * Data = WeaponDataHandle.GetRow<FWeaponData>("No data");
  checkf(Data, TEXT("The table isn't assigned"));

  Sounds         = &Data->SoundsSet;
  HitboxDamage   = &Data->HitboxDamage;
  RecoilParams   = &Data->RecoilParams;
  FireModes      = &Data->FireModes;
  FireRate       = Data->FireRate;
  MagazineAmount = Data->MagazineAmount;
}

void AWeapon::Reload()
{
  Server_Reload();
}

bool AWeapon::IsReloading() const
{
  return bIsReloading;
}

bool AWeapon::IsMagazineExtracted() const
{
  return CurrentMagAmount == INVALID_AMMO_AMOUNT;
}

USoundBase * AWeapon::GetSound(EWeaponSound SoundType) const
{
  FSoundsSet * SoundsSet = Sounds->Find(SoundType);
  if (!SoundsSet || SoundsSet->Sounds.IsEmpty())
    return nullptr;

  return SoundsSet->Sounds[FMath::RandRange(0, SoundsSet->Sounds.Num() - 1)].Get();
}

void AWeapon::SwitchFireMode()
{
  Server_SwitchFireMode();
}

int32 AWeapon::GetDamage(const TSoftObjectPtr<class UPhysicalMaterial> & MaterialHit) const
{
  ensure(HasAuthority());

  const int32 * Damage = HitboxDamage->Find(MaterialHit);
  if (!Damage)
  {
    UE_LOG(LogCore, Warning, TEXT("No damage set for %s"), *MaterialHit->GetName());
    return 0;
  }

  return *Damage + FMath::RandRange(-*Damage / 10, *Damage / 10);
}

void AWeapon::OnMagazineExtracted()
{
  if (HasAuthority())
  {
    CurrentMagAmount = INVALID_AMMO_AMOUNT;
    Client_MagAmountChanged(CurrentMagAmount);
  }

  //verify(m_MagazineActor);
  //UStaticMeshComponent * MagazineMesh = m_MagazineActor->GetStaticMeshComponent();
  //
  //const FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, true);
  //MagazineMesh->DetachFromComponent(DetachmentRules);
  //MagazineMesh->SetSimulatePhysics(true);
  //
  //m_MagazineActor->SetLifeSpan(30);
  //m_MagazineActor = nullptr;
  //
  //PlaySound(EWeaponSound::Reload_1);
}

void AWeapon::OnMagazineTaken()
{
  //auto HeroMesh = GetOwner<ACharacter>()->GetMesh();
  //
  //const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
  //
  //m_MagazineActor = GetWorld()->SpawnActor<AStaticMeshActor>(MagazineClass);
  //m_MagazineActor->AttachToComponent(HeroMesh, AttachmentRules, TEXT("magazine_socket"));
  //
  //PlaySound(EWeaponSound::Reload_2);
}

void AWeapon::OnMagazineInserted()
{
  if (HasAuthority())
  {
    CurrentMagAmount = MagazineAmount;
    bIsReloading = false;
    Client_MagAmountChanged(CurrentMagAmount);
  }
}

bool AWeapon::IsAmmo() const
{
  return CurrentMagAmount > 0;
}

const FWeaponRecoilParams & AWeapon::GetRecoil() const
{
  return *RecoilParams;
}

bool AWeapon::CanBeShoot() const
{
  return IsAmmo() && !IsReloading();
}

void AWeapon::ApplyFireModeAmmoLimit()
{
  static const TMap<EWeaponFireMode, uint32> AmmoToShoot{
      {EWeaponFireMode::Single,    1u},
      {EWeaponFireMode::Burst,     3u},
      {EWeaponFireMode::Automatic, MAX_uint32},
    };

  LimitAmmoToShoot = AmmoToShoot[FireMode];
}

void AWeapon::ResetFireModeAmmoLimit()
{
  ApplyFireModeAmmoLimit();
}

void AWeapon::Fire()
{
  ensure(HasAuthority());

  if (!IsAmmo())
  {
    OnNoAmmoLeft();
    StopShooting();
    return;
  }

  if (--LimitAmmoToShoot == 0)
    StopShooting();

  --CurrentMagAmount;
  Client_MagAmountChanged(CurrentMagAmount);
  ShootProjectile();
  Multicast_PlayShootAnimation();
  Client_WeaponShoot(GetRecoil());
}

void AWeapon::ShootProjectile()
{
  ensure(HasAuthority());

  auto Hero = GetOwner<AHeroBase>();

  ABulletProjectile * Projectile;
  FTransform          ProjectileTransform;
  FVector             Direction;

  FActorSpawnParameters SpawnParams;
  SpawnParams.Owner      = this;
  SpawnParams.Instigator = GetInstigator();

  if (Hero->IsWeaponAiming())
  {
    auto Camera = Hero->GetComponentByClass<UCameraComponent>();
    auto Arm    = Hero->GetComponentByClass<USpringArmComponent>();

    ProjectileTransform = FTransform(Arm->GetComponentRotation(), Arm->GetComponentLocation());
    Direction           = Camera->GetForwardVector();
  }
  else
  {
    const FTransform BarrelTransform(ModelMesh->GetSocketTransform(TEXT("Barrel")));

    ProjectileTransform = FTransform(BarrelTransform.GetRotation(), BarrelTransform.GetLocation());
    Direction           = BarrelTransform.GetRotation().GetForwardVector();
  }

  Projectile = GetWorld()->SpawnActor<ABulletProjectile>(ProjectileClass, ProjectileTransform, SpawnParams);
  if (Projectile)
    Projectile->SetDirection(Direction);
}

void AWeapon::PlaySound(EWeaponSound SoundType)
{
  if (!HasAuthority())
  {
    USoundBase * Sound = GetSound(SoundType);
    if (!Sound)
    {
      UE_LOG(LogCore, Warning, TEXT("No sounds set for %s"), *GetDisplayName().ToString());
      return;
    }

    UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, GetActorLocation());
  }
  else
  {
    Client_PlaySound(SoundType);
  }
}

//
// Replication
//

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME_CONDITION(AWeapon, bIsShooting,      COND_None);
  DOREPLIFETIME_CONDITION(AWeapon, bIsReloading,     COND_None);
  DOREPLIFETIME_CONDITION(AWeapon, FireMode,         COND_None);
  DOREPLIFETIME_CONDITION(AWeapon, CurrentMagAmount, COND_None);
  DOREPLIFETIME_CONDITION(AWeapon, LimitAmmoToShoot, COND_OwnerOnly);
}

void AWeapon::Client_PlaySound_Implementation(EWeaponSound SoundType)
{
  PlaySound(SoundType);
}

void AWeapon::Server_Reload_Implementation()
{
  StopShooting();
  bIsReloading = true;
  Multicast_PlayReloadAnimation();
}

void AWeapon::Multicast_PlayReloadAnimation_Implementation()
{
  PlayReloadAnimation();
}

void AWeapon::Multicast_PlayShootAnimation_Implementation()
{
  PlayShootAnimation();
}

void AWeapon::Server_StartShooting_Implementation()
{
  bIsShooting = true;
  ApplyFireModeAmmoLimit();

  GetWorldTimerManager().SetTimer(m_ShootingTimer, this, &AWeapon::Fire, FireRate, true, 0.0f);
}

bool AWeapon::Server_StartShooting_Validate()
{
  return IsAmmo();
}

void AWeapon::Server_StopShooting_Implementation()
{
  bIsShooting = false;
  GetWorldTimerManager().ClearTimer(m_ShootingTimer);
  ResetFireModeAmmoLimit();
}

void AWeapon::Client_WeaponShoot_Implementation(const FWeaponRecoilParams & Recoil)
{
  OnWeaponShoot.Broadcast(Recoil);
}

void AWeapon::Server_SwitchFireMode_Implementation()
{
  const int32 Index = FireModes->Find(FireMode);

  const EWeaponFireMode PrevMode = FireMode;
  const EWeaponFireMode NextMode = (*FireModes)[(Index + 1) % FireModes->Num()];

  if (NextMode != PrevMode)
  {
    FireMode = NextMode;
    Client_FireModeChanged(FireMode);
  }
}

void AWeapon::Client_FireModeChanged_Implementation(EWeaponFireMode Mode)
{
  PlaySound(EWeaponSound::FireModeSwitch);
  OnFireModeChanged.Broadcast(Mode);
}

void AWeapon::Client_MagAmountChanged_Implementation(int32 MagAmount)
{
  OnWeaponAmmoChanged.Broadcast(MagAmount);
}