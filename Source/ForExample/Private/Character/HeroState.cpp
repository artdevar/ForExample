#include "Character/HeroState.h"
#include "Character/HeroBase.h"
#include "Net/UnrealNetwork.h"

void AHeroState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME(AHeroState, Health);
}

void AHeroState::Reset()
{
  Health = MaxHealth;

  Super::Reset();
}

void AHeroState::DecreaseHealth(int32 Amount)
{
  Health -= Amount;

  if (Health < 0)
  {
    Health = 0;
    GetOwner<AHeroBase>()->OnNoHealthLeft();
  }
}