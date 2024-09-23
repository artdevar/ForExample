#include "Components/InteractableActor.h"

EInteractableType AInteractableActor::GetType() const
{
  return Type;
}

bool AInteractableActor::IsPickedUp() const
{
  return bIsPickedUp;
}

bool AInteractableActor::IsPickupable() const
{
  return bIsPickupable && !IsPickedUp();
}

bool AInteractableActor::IsDroppable() const
{
  return true;
}

void AInteractableActor::SetPickupable(bool IsPickupable)
{
  bIsPickupable = IsPickupable;
}

void AInteractableActor::OnPickedUp()
{
  bIsPickedUp = true;
}

void AInteractableActor::OnDropped()
{
  bIsPickedUp = false;
}

const FName & AInteractableActor::GetDisplayName() const
{
  return DisplayName;
}