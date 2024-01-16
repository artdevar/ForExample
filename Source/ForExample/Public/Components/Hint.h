#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Containers/Map.h"
#include "Internationalization/Text.h"
#include "Hint.generated.h"

class AHeroBase;
class APickupableActor;
class UInputAction;

UENUM(BlueprintType)
enum class EHintAction : uint8
{
  None   UMETA(DisplayName=None),
  Pickup UMETA(DisplayName=Pickup)
};

USTRUCT(BlueprintType)
struct FActionText
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere)
  FText Text;

  UPROPERTY(EditAnywhere)
  UInputAction * Action;
};

UCLASS()
class FOREXAMPLE_API AHint : public AActor
{
  GENERATED_BODY()

public: 

  AHint();

  void Tick(float DeltaSeconds) override;

protected: 

  UFUNCTION(BlueprintImplementableEvent)
  void SetText(const FText & Text);

  UFUNCTION(BlueprintImplementableEvent)
  void SetTextOpacity(float Opacity);

  void UpdateText();

  void FaceToHero();

  void UpdatePosition();

  void UpdateOpacity();

  bool IsNeedFacingToOwner() const;

  bool IsNeedAttachToPickupable() const;

protected: 

  UPROPERTY(EditDefaultsOnly)
  TMap<EHintAction, FActionText> ActionTextWithBind;

public:
  
  EHintAction HintAction = EHintAction::None;
  AActor * Pickupable = nullptr;

  bool NeedFacing             = false;
  bool IsAttachedToPickupable = false;

  float DistanceDiscoverableSquared = 0.0f;
};
