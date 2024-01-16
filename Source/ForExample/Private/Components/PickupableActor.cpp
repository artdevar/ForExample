#include "Components/PickupableActor.h"

EPickupableType APickupableActor::GetType() const
{
  return Type;
}

bool APickupableActor::IsPickedUp() const
{
  return m_IsPickedUp;
}

bool APickupableActor::IsPickupable() const
{
  return m_IsPickupable && !IsPickedUp();
}

void APickupableActor::SetPickupable(bool IsPickupable)
{
  m_IsPickupable = IsPickupable;
}

void APickupableActor::OnPickedUp()
{
  m_IsPickedUp = true;
}

void APickupableActor::OnDropped()
{
  m_IsPickedUp = false;
}
