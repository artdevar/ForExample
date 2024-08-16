#include "Weapon/Weapon.h"
#include "Weapon/BulletProjectile.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Character/HeroBase.h"
#include "Components/MeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Game/MyGameInstance.h"
#include "Kismet/GameplayStatics.h"

const int32 AWeapon::INVALID_AMMO_AMOUNT = -1;

AWeapon::AWeapon()
{
  PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::BeginPlay()
{
  Super::BeginPlay();

  GameInstance = GetWorld()->GetGameInstance<UMyGameInstance>();

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
    auto Camera = reinterpret_cast<UCameraComponent*>(Hero->GetComponentByClass(UCameraComponent::StaticClass()));
    auto Arm    = reinterpret_cast<USpringArmComponent*>(Hero->GetComponentByClass(USpringArmComponent::StaticClass()));

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
  {
    Projectile->SetDirection(Direction);
    Projectile->SetDamage(GetDamage());
  }

  SpawnShootProjectile();
}

void AWeapon::OnNoAmmoLeft()
{
  PlaySound(EWeaponSound::NoAmmo);
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
  UGameplayStatics::PlaySoundAtLocation(GetWorld(), GameInstance->GetRandomWeaponSound(GetClass(), SoundType), GetActorLocation());
}

void AWeapon::SwitchFireMode()
{
  const auto PrevMode = FireMode;

  const int Index = FireModes.Find(FireMode);
  FireMode = FireModes[(Index + 1) % FireModes.Num()];

  if (FireMode != PrevMode)
  {
    PlaySound(EWeaponSound::FireModeSwitch);
    OnFireModeChanged.Broadcast(FireMode);
  }
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
}

void AWeapon::OnMagazineTaken()
{
  auto HeroMesh = GetOwner<ACharacter>()->GetMesh();

  const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);

  m_MagazineActor = GetWorld()->SpawnActor<AStaticMeshActor>(MagazineClass);
  m_MagazineActor->AttachToComponent(HeroMesh, AttachmentRules, TEXT("magazine_socket"));
}

void AWeapon::OnMagazineInserted()
{
  const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
  m_MagazineActor->AttachToComponent(ModelMesh, AttachmentRules, TEXT("Magazine"));

  CurrentMagazineAmount = MagazineAmount;
  OnWeaponAmmoChanged.Broadcast(CurrentMagazineAmount);
  m_IsReloading = false;
}

bool AWeapon::IsAmmo() const
{
  return CurrentMagazineAmount > 0;
}

FWeaponRecoilParams AWeapon::GetRecoil() const
{
  return RecoilParams;
}

int AWeapon::GetDamage() const
{
  return BaseDamage + FMath::RandRange(-3, 3);
}

bool AWeapon::CanBeShoot() const
{
  return IsAmmo() && !IsReloading();
}

void AWeapon::ApplyFireModeAmmoLimit()
{
  constexpr uint32 AmmoToShoot[] = {1, 3};

  if (FireMode != EWeaponFireMode::Automatic)
    m_AmountAmmoToShoot = AmmoToShoot[int(FireMode)];
}

void AWeapon::ResetFireModeAmmoLimit()
{
  m_AmountAmmoToShoot.reset();
}