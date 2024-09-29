#include "Character/HeroState.h"
#include "Character/HeroBase.h"
#include "Net/UnrealNetwork.h"

AHeroState::AHeroState()
{
  bReplicates = true;
}

void AHeroState::Reset()
{
  Health = MaxHealth;

  Super::Reset();
}

void AHeroState::IncreaseHealth(int32 Amount)
{
  ensure(HasAuthority());

  Health = FMath::Min(MaxHealth, Health + Amount);
}

void AHeroState::DecreaseHealth(int32 Amount)
{
  ensure(HasAuthority());

  Health = FMath::Max(0, Health - Amount);
}

//
// Replication
//

void AHeroState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
  Super::GetLifetimeReplicatedProps(OutLifetimeProps);

  DOREPLIFETIME_CONDITION(AHeroState, MaxHealth, COND_OwnerOnly);
  DOREPLIFETIME_CONDITION(AHeroState, Health,    COND_OwnerOnly);
}

void AHeroState::OnRep_HealthChanged()
{
  GetPawn<AHeroBase>()->OnHealthPointsChanged();
}
