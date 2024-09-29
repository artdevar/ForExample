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

UCLASS(Abstract)
class FOREXAMPLE_API AInteractableActor : public AActor
{
  GENERATED_BODY()

public:

  void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const override;

  const FName & GetDisplayName() const;

  UFUNCTION(BlueprintPure)
  EInteractableType GetType() const;

  UFUNCTION(BlueprintPure)
  bool IsPickedUp() const;

  UFUNCTION(BlueprintPure)
  bool IsPickupable() const;

  UFUNCTION(BlueprintPure)
  virtual bool IsDroppable() const;

  UFUNCTION(Server, Reliable)
  void SetPickupable(bool IsPickupable);

  UFUNCTION(Server, Reliable)
  virtual void OnPickedUp();

  UFUNCTION(Server, Reliable)
  virtual void OnDropped();

protected:

  UPROPERTY(EditDefaultsOnly)
  FName DisplayName;

  UPROPERTY(EditDefaultsOnly)
  EInteractableType Type = EInteractableType::None;

  UPROPERTY(Replicated)
  bool bIsPickedUp = false;

  UPROPERTY(Replicated)
  bool bIsPickupable = true;

};
