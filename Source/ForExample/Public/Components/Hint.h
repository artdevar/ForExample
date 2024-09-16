#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Containers/Map.h"
#include "Internationalization/Text.h"
#include "Hint.generated.h"

class AHeroBase;
class AInteractableActor;
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

  void UpdatePosition(float DeltaSeconds);

  void UpdateOpacity();

  bool IsNeedFacingToOwner() const;

  bool IsNeedAttachToInteractable() const;

protected:

  UPROPERTY(EditDefaultsOnly)
  TMap<EHintAction, FActionText> ActionTextWithBind;

  UPROPERTY(EditDefaultsOnly)
  FVector PositionDelta;

  UPROPERTY(EditDefaultsOnly)
  float FullOpacityDistanceSquared = 0.0f;

  UPROPERTY(EditDefaultsOnly)
  float InterpolationSpeed = 0.0f;

public:

  EHintAction HintAction = EHintAction::None;
  AInteractableActor * Interactable = nullptr;

  bool NeedFacing               = false;
  bool IsAttachedToInteractable = false;

  float DistanceDiscoverableSquared = 0.0f;
};
