#include "Components/Hint.h"
#include "Character/HeroBase.h"
#include "Components/InteractableActor.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utils/Input.h"

AHint::AHint()
{
  PrimaryActorTick.bCanEverTick = true;
}

void AHint::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);

  if (GetOwner())
  {
    UpdateText();
    UpdateOpacity();
  }

  if (IsNeedFacingToOwner())
    FaceToHero();

  if (IsNeedAttachToInteractable())
    UpdatePosition();
}

void AHint::UpdateText()
{
  const FActionText * ActionText = ActionTextWithBind.Find(HintAction);

  APlayerController * PlayerController = GetOwner<ACharacter>()->GetController<APlayerController>();
  const FText Bind = UInput::GetKeyBindedToAction(PlayerController, ActionText->Action);

  SetText(FText::Format(ActionText->Text, Interactable->GetActorName(), Bind));
}

void AHint::FaceToHero()
{
  const FVector HintLocation = GetActorLocation();
  const auto    HeroCamera   = reinterpret_cast<UCameraComponent*>(GetOwner()->GetComponentByClass(UCameraComponent::StaticClass()));

  const FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(HintLocation, HeroCamera->GetComponentLocation());
  SetActorRotation(Rotation);
}

void AHint::UpdatePosition()
{
  const FVector PickupablePos = Interactable->GetActorLocation();
  SetActorLocation(PickupablePos);
}

void AHint::UpdateOpacity()
{
  const FVector HeroLocation = GetOwner()->GetActorLocation();
  const FVector HintLocation = GetActorLocation();

  const float DistanceBetweenSquared = (HeroLocation - HintLocation).SquaredLength();
  const float NewOpacity = (DistanceDiscoverableSquared - DistanceBetweenSquared) / (DistanceDiscoverableSquared - (75.0f * 75.0f));

  SetTextOpacity(FMath::Clamp(NewOpacity, 0.0f, 1.0f));
}

bool AHint::IsNeedFacingToOwner() const
{
  return NeedFacing && GetOwner();
}

bool AHint::IsNeedAttachToInteractable() const
{
  return IsAttachedToInteractable && Interactable;
}
