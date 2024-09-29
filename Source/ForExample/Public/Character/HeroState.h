#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "HeroState.generated.h"

UCLASS()
class FOREXAMPLE_API AHeroState : public APlayerState
{
  GENERATED_BODY()

public:

  AHeroState();

  void Reset() override;

  void IncreaseHealth(int32 Amount);

  void DecreaseHealth(int32 Amount);

protected:

//
// Replication
//

  void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const override;

  UFUNCTION()
  void OnRep_HealthChanged();

public:

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated)
  int32 MaxHealth = 0;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, ReplicatedUsing=OnRep_HealthChanged)
  int32 Health = 0;

};
