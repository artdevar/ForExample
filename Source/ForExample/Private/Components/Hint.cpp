#include "Components/Hint.h"
#include "Character/HeroBase.h"
#include "Components/InteractableActor.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utils/Input.h"

AHint::AHint()
{
  PrimaryActorTick.bCanEverTick = true;
  bReplicates = false;
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
    UpdatePosition(DeltaSeconds);
}

void AHint::UpdateText()
{
  const FActionText * ActionText = ActionTextWithBind.Find(HintAction);

  APlayerController * PlayerController = GetOwner<ACharacter>()->GetController<APlayerController>();
  const FText Bind = UInput::GetKeyBindedToAction(PlayerController, ActionText->Action);

  SetText(FText::Format(ActionText->Text, Bind, FText::FromName(Interactable->GetDisplayName())));
}

void AHint::FaceToHero()
{
  const FVector HintLocation = GetActorLocation();
  const auto    HeroCamera   = GetOwner()->GetComponentByClass<UCameraComponent>();

  const FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(HintLocation, HeroCamera->GetComponentLocation());
  SetActorRotation(Rotation);
}

void AHint::UpdatePosition(float DeltaSeconds)
{
  const FVector CurrentLocation = GetActorLocation();
  const FVector TargetLocation  = Interactable->GetActorLocation() + PositionDelta;

  SetActorLocation(FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaSeconds, InterpolationSpeed));
}

void AHint::UpdateOpacity()
{
  const FVector HeroLocation = GetOwner()->GetActorLocation();
  const FVector HintLocation = GetActorLocation();

  const float DistanceBetweenSquared = (HeroLocation - HintLocation).SquaredLength();
  const float NewOpacity = (DistanceDiscoverableSquared - DistanceBetweenSquared) / FullOpacityDistanceSquared;

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
