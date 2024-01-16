#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupableActor.generated.h"

UENUM(BlueprintType)
enum class EPickupableType : uint8
{
  None   UMETA(DisplayName=None),
  Weapon UMETA(DisplayName=Weapon)
};

UCLASS(Abstract)
class APickupableActor : public AActor
{
  GENERATED_BODY()

public:

  UFUNCTION(BlueprintPure, Category=Pickupable)
  EPickupableType GetType() const;

  UFUNCTION(BlueprintPure, Category=Pickupable)
  bool IsPickedUp() const;

  UFUNCTION(BlueprintPure, Category=Pickupable)
  bool IsPickupable() const;

  UFUNCTION(BlueprintCallable, Category=Pickupable)
  void SetPickupable(bool IsPickupable);

  UFUNCTION(BlueprintCallable, Category=Pickupable)
  void OnPickedUp();

  UFUNCTION(BlueprintCallable, Category=Pickupable)
  void OnDropped();

protected:

  UPROPERTY(EditDefaultsOnly, Category=Pickupable)
  EPickupableType Type = EPickupableType::None;

  bool m_IsPickedUp   = false;
  bool m_IsPickupable = true;

};
