#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Shared/Weapons.h"
#include "HeroBase.generated.h"

class AWeapon;
class AHint;
class AInteractableActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponPickedUpSignature, AWeapon*, Weapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWeaponDroppedSignature, AWeapon*, Weapon);

UCLASS()
class FOREXAMPLE_API AHeroBase : public ACharacter
{
  GENERATED_BODY()

public:

  AHeroBase();

  void AddControlRotation(const FRotator & NewRotation);

  UFUNCTION(BlueprintPure, Category=Weapon)
  bool HasWeapon() const;

  UFUNCTION(BlueprintPure, Category=Weapon)
  bool IsWeaponAiming() const;

  UFUNCTION(BlueprintPure, Category=Weapon)
  bool IsWeaponReloading() const;

  void OnNoHealthLeft();

protected:

  void Tick(float DeltaSeconds) override;

  void Reset() override;

  void BeginPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) override;

  bool CanJumpInternal_Implementation() const override;

  bool CanCrouch() const override;

  UFUNCTION(BlueprintCallable)
  void InputActionUse();

  UFUNCTION(BlueprintCallable)
  void InputActionDrop();

  UFUNCTION(BlueprintCallable)
  void InputActionAim();

  UFUNCTION(BlueprintCallable)
  void InputActionFinishedAim();

  UFUNCTION(BlueprintCallable)
  void InputActionAttack();

  UFUNCTION(BlueprintCallable)
  void InputActionFinishedAttack();

  UFUNCTION(BlueprintCallable)
  void InputActionRun();

  UFUNCTION(BlueprintCallable)
  void InputActionFinishedRun();

  UFUNCTION(BlueprintCallable)
  void InputActionCrouch();

  UFUNCTION(BlueprintCallable)
  void InputActionFinishedCrouch();

  UFUNCTION(BlueprintCallable)
  void InputActionWalk();

  UFUNCTION(BlueprintCallable)
  void InputActionFinishedWalk();

  UFUNCTION(BlueprintCallable)
  void InputActionReload();

  UFUNCTION(BlueprintCallable)
  void InputActionSwitchFireMode();

  UFUNCTION(BlueprintCallable, Category=Weapon)
  void DropWeapon();

  UPROPERTY(BlueprintAssignable, Category=Weapon)
  FWeaponPickedUpSignature OnWeaponPickedUp;

  UPROPERTY(BlueprintAssignable, Category=Weapon)
  FWeaponDroppedSignature OnWeaponDropped;

  void SetWalkingSpeed(float WalkSpeed);

  UFUNCTION(BlueprintPure)
  bool IsRunning() const;

  UFUNCTION(BlueprintCallable)
  void OnWeaponShoot(FWeaponRecoilParams RecoilParams);

  UFUNCTION(BlueprintImplementableEvent, Category=Weapon)
  void PlayWeaponReloadAnimation();

  UFUNCTION(BlueprintImplementableEvent, Category=Weapon)
  void PlayWeaponShootAnimation();

  UFUNCTION(BlueprintImplementableEvent)
  void InitInput();

  AInteractableActor * GetClosestInteractable() const;

  UFUNCTION(BlueprintCallable)
  void TryCreateHint();

  void TryDestroyHint();

protected:

  UPROPERTY(EditDefaultsOnly, Category=Movement)
  float SlowWalkSpeed = 0.0f;

  UPROPERTY(EditDefaultsOnly, Category=Movement)
  float WalkSpeed = 0.0f;

  UPROPERTY(EditDefaultsOnly, Category=Movement)
  float RunSpeed = 0.0f;

  UPROPERTY(EditDefaultsOnly, Category=Interactable)
  float InteractableDiscoverDistance = 0.0f;

  UPROPERTY(EditDefaultsOnly, Category=Interactable)
  float DropImpulse = 0.0f;

  UPROPERTY(EditDefaultsOnly, Category=Projectile)
  TSubclassOf<class AHint> HintClass;

  UPROPERTY(BlueprintReadOnly, Category=Weapon)
  AWeapon * Weapon = nullptr;

  UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Weapon)
  bool bApplyControllerRotationYawWithWeapon = false;

  UPROPERTY(BlueprintReadOnly)
  bool bIsAiming = false;

protected:

  bool    bLookForInteractables = false;
  AHint * HintToInteractable    = nullptr;

};
