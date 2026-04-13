#include "Components/Inventory.h"

UInventory::UInventory()
{
  PrimaryComponentTick.bCanEverTick = true;
  SetIsReplicatedByDefault(true);
}

void UInventory::BeginPlay()
{
  Super::BeginPlay();
}

void UInventory::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

