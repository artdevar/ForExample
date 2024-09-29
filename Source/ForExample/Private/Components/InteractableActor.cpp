#include "Components/InteractableActor.h"
#include "Net/UnrealNetwork.h"

void AInteractableActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(AInteractableActor, bIsPickedUp);
  DOREPLIFETIME(AInteractableActor, bIsPickupable);
}

const FName & AInteractableActor::GetDisplayName() const
{
  return DisplayName;
}

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
  return IsPickedUp();
}

void AInteractableActor::SetPickupable_Implementation(bool IsPickupable)
{
  bIsPickupable = IsPickupable;
}

void AInteractableActor::OnPickedUp_Implementation()
{
  bIsPickedUp = true;
}

void AInteractableActor::OnDropped_Implementation()
{
  bIsPickedUp = false;
}