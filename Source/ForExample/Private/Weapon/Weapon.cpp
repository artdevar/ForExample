#include "Weapon/Weapon.h"
#include "Weapon/BulletProjectile.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Character/HeroBase.h"
#include "Components/MeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Data/WeaponData.h"
#include "Kismet/GameplayStatics.h"

const int32 AWeapon::INVALID_AMMO_AMOUNT = -1;

AWeapon::AWeapon()
{
  PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::BeginPlay()
{
  Super::BeginPlay();

  InitFromTable();

  FireMode = FireModes->Contains(EWeaponFireMode::Automatic) ? EWeaponFireMode::Automatic :
             FireModes->Contains(EWeaponFireMode::Burst)     ? EWeaponFireMode::Burst     : EWeaponFireMode::Single;

  const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);

  m_MagazineActor = GetWorld()->SpawnActor<AStaticMeshActor>(MagazineClass);
  m_MagazineActor->AttachToComponent(ModelMesh, AttachmentRules, TEXT("Magazine"));

  CurrentMagazineAmount = MagazineAmount;
  OnWeaponAmmoChanged.Broadcast(CurrentMagazineAmount);
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

  m_IsShooting = true;
  ApplyFireModeAmmoLimit();

  GetWorldTimerManager().SetTimer(m_ShootingTimer, this, &AWeapon::Shoot, FireRate, true);
  Shoot();
}

void AWeapon::StopShooting()
{
  GetWorldTimerManager().ClearTimer(m_ShootingTimer);

  m_IsShooting = false;
  ResetFireModeAmmoLimit();
}

void AWeapon::Shoot()
{
  if (!IsAmmo())
  {
    OnNoAmmoLeft();
    StopShooting();
    return;
  }

  if (m_AmountAmmoToShoot.has_value() && --m_AmountAmmoToShoot.value() == 0)
    StopShooting();

  --CurrentMagazineAmount;
  ShootProjectile();
  OnWeaponShoot.Broadcast(GetRecoil());
  OnWeaponAmmoChanged.Broadcast(CurrentMagazineAmount);
}

void AWeapon::ShootProjectile()
{
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

  SpawnShootProjectile();
}

void AWeapon::OnNoAmmoLeft()
{
  PlaySound(EWeaponSound::NoAmmo);
}

void AWeapon::InitFromTable()
{
  FWeaponData * Data = WeaponDataHandle.GetRow<FWeaponData>("No data");
  verify(Data);

  Sounds         = &Data->SoundsSet;
  HitboxDamage   = &Data->HitboxDamage;
  RecoilParams   = &Data->RecoilParams;
  FireModes      = &Data->FireModes;
  FireRate       = Data->FireRate;
  MagazineAmount = Data->MagazineAmount;
}

void AWeapon::Reload()
{
  StopShooting();
  m_IsReloading = true;
}

bool AWeapon::IsReloading() const
{
  return m_IsReloading;
}

void AWeapon::PlaySound(EWeaponSound SoundType)
{
  USoundBase * Sound = GetSound(SoundType);
  if (!Sound)
  {
    UE_LOG(LogCore, Warning, TEXT("No sounds set for %s"), *GetDisplayName().ToString());
    return;
  }

  UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, GetActorLocation());
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
  const auto PrevMode = FireMode;

  const int Index = FireModes->Find(FireMode);
  FireMode = (*FireModes)[(Index + 1) % FireModes->Num()];

  if (FireMode != PrevMode)
  {
    PlaySound(EWeaponSound::FireModeSwitch);
    OnFireModeChanged.Broadcast(FireMode);
  }
}

int32 AWeapon::GetDamage(const TSoftObjectPtr<class UPhysicalMaterial> & MaterialHit) const
{
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
  CurrentMagazineAmount = INVALID_AMMO_AMOUNT;
  OnWeaponAmmoChanged.Broadcast(CurrentMagazineAmount);

  verify(m_MagazineActor);
  UStaticMeshComponent * MagazineMesh = m_MagazineActor->GetStaticMeshComponent();

  const FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, true);
  MagazineMesh->DetachFromComponent(DetachmentRules);
  MagazineMesh->SetSimulatePhysics(true);

  m_MagazineActor->SetLifeSpan(30);
  m_MagazineActor = nullptr;

  PlaySound(EWeaponSound::Reload_1);
}

void AWeapon::OnMagazineTaken()
{
  auto HeroMesh = GetOwner<ACharacter>()->GetMesh();

  const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);

  m_MagazineActor = GetWorld()->SpawnActor<AStaticMeshActor>(MagazineClass);
  m_MagazineActor->AttachToComponent(HeroMesh, AttachmentRules, TEXT("magazine_socket"));

  PlaySound(EWeaponSound::Reload_2);
}

void AWeapon::OnMagazineInserted()
{
  const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
  m_MagazineActor->AttachToComponent(ModelMesh, AttachmentRules, TEXT("Magazine"));

  CurrentMagazineAmount = MagazineAmount;
  OnWeaponAmmoChanged.Broadcast(CurrentMagazineAmount);
  m_IsReloading = false;

  PlaySound(EWeaponSound::Reload_3);
}

bool AWeapon::IsAmmo() const
{
  return CurrentMagazineAmount > 0;
}

FWeaponRecoilParams AWeapon::GetRecoil() const
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

  m_AmountAmmoToShoot = AmmoToShoot[FireMode];
}

void AWeapon::ResetFireModeAmmoLimit()
{
  m_AmountAmmoToShoot.reset();
}