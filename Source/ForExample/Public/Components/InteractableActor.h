#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableActor.generated.h"

UENUM(BlueprintType)
enum class EInteractableType : uint8
{
  None   UMETA(DisplayName = None),
  Weapon UMETA(DisplayName = Weapon)
};

UCLASS()
class FOREXAMPLE_API AInteractableActor : public AActor
{
  GENERATED_BODY()

public:

  UFUNCTION(BlueprintPure)
  EInteractableType GetType() const;

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

  const FName & GetDisplayName() const;

protected:

  UPROPERTY(EditDefaultsOnly)
  FName DisplayName;

  UPROPERTY(EditDefaultsOnly, Category=Pickupable)
  EInteractableType Type = EInteractableType::None;

  bool m_IsPickedUp = false;
  bool m_IsPickupable = true;

};
