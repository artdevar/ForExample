#include "Weapon/Weapon.h"
#include "Weapon/BulletProjectile.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Character/HeroBase.h"
#include "Components/MeshComponent.h"
#include "Engine/StaticMeshActor.h"

const int32 AWeapon::INVALID_AMMO_AMOUNT = -1;

AWeapon::AWeapon()
{
  PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::BeginPlay()
{
  Super::BeginPlay();

  const FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);

  m_MagazineActor = GetWorld()->SpawnActor<AStaticMeshActor>(MagazineClass);
  m_MagazineActor->AttachToComponent(ModelMesh, AttachmentRules, TEXT("Magazine"));

  CurrentMagazineAmount = MagazineAmount;
  OnWeaponAmmoChanged.Broadcast(CurrentMagazineAmount);
}

void AWeapon::StartShooting()
{
  m_IsShooting = true;
  m_NoAmmoEventTriggered = false;

  Shoot();
  GetWorldTimerManager().SetTimer(m_ShootingTimer, this, &AWeapon::Shoot, FireRate, true);
}

void AWeapon::StopShooting()
{
  m_IsShooting = false;
  m_NoAmmoEventTriggered = false;

  GetWorldTimerManager().ClearTimer(m_ShootingTimer);
}

void AWeapon::Shoot()
{
  if (IsReloading())
  {
    StopShooting();
    return;
  }

  if (!IsAmmo())
  {
    OnNoAmmoLeft();
    StopShooting();
    return;
  }

  --CurrentMagazineAmount;
  PlaySound(EWeaponSound::Shoot);
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
}

void AWeapon::OnNoAmmoLeft()
{
  if (m_NoAmmoEventTriggered)
    return;

  PlaySound(EWeaponSound::NoAmmo);
  m_NoAmmoEventTriggered = true;
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

FVector2D AWeapon::GetRecoil() const
{
  const int Index = MagazineAmount - CurrentMagazineAmount - 1;
  return Recoil[Index % Recoil.Num()];
}

int AWeapon::GetDamage() const
{
  return BaseDamage + FMath::RandRange(-3, 3);
}

bool AWeapon::CanBeShoot() const
{
  return IsAmmo() && !IsReloading();
}
