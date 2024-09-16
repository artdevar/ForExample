#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "HeroState.generated.h"

UCLASS()
class FOREXAMPLE_API AHeroState : public APlayerState
{
  GENERATED_BODY()

public:

  void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const override;

  void Reset() override;

  void DecreaseHealth(int32 Amount);

public:

  UPROPERTY(BlueprintReadWrite, EditAnywhere)
  int32 MaxHealth = 0;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Replicated)
  int32 Health = 0;

};
