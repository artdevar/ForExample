#include "Components/InteractableActor.h"

EInteractableType AInteractableActor::GetType() const
{
  return Type;
}

bool AInteractableActor::IsPickedUp() const
{
  return m_IsPickedUp;
}

bool AInteractableActor::IsPickupable() const
{
  return m_IsPickupable && !IsPickedUp();
}

void AInteractableActor::SetPickupable(bool IsPickupable)
{
  m_IsPickupable = IsPickupable;
}

void AInteractableActor::OnPickedUp()
{
  m_IsPickedUp = true;
}

void AInteractableActor::OnDropped()
{
  m_IsPickedUp = false;
}

const FText & AInteractableActor::GetActorName() const
{
  return Name;
}