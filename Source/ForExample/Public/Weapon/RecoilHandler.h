#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Shared/Weapons.h"
#include <queue>
#include "RecoilHandler.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FOREXAMPLE_API URecoilHandler : public UActorComponent
{
  GENERATED_BODY()

public:

  URecoilHandler();

  void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction) override;

  void Add(const FWeaponRecoilParams & RecoilParams);

protected:

  static FRotator Interp(const FRotator & Current, const FRotator & Target, float Alpha);

protected:

  FWeaponRecoilParams m_RecoilParams;
  float               m_TimeSinceShoot = 0.0f;

  FRotator m_TargetRotation;
  FRotator m_CurrentRotation;

};
