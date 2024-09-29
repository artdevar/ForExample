#include "Weapon/RecoilHandler.h"
#include "Character/HeroBase.h"
#include "Camera/CameraComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

URecoilHandler::URecoilHandler() :
  m_TimeSinceShoot(0.0f),
  m_TargetRotation(FRotator::ZeroRotator),
  m_CurrentRotation(FRotator::ZeroRotator)
{
  PrimaryComponentTick.bCanEverTick = true;
  SetIsReplicatedByDefault(false);
}

void URecoilHandler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
  Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

  m_TargetRotation  = Interp(m_TargetRotation,  FRotator::ZeroRotator, m_TimeSinceShoot += DeltaTime * m_RecoilParams.RecoilSmoothness);
  m_CurrentRotation = Interp(m_CurrentRotation, m_TargetRotation,      m_TimeSinceShoot  = DeltaTime * m_RecoilParams.RecoilStopSpeed);

  GetOwner<AHeroBase>()->AddControlRotation(m_CurrentRotation);
}

void URecoilHandler::Add(const FWeaponRecoilParams & RecoilParams)
{
  m_RecoilParams   = RecoilParams;
  m_TimeSinceShoot = 0.0f;
  m_TargetRotation.Add(
      FMath::RandRange(RecoilParams.RecoilRangeY.X, RecoilParams.RecoilRangeY.Y),
      FMath::RandRange(RecoilParams.RecoilRangeX.X, RecoilParams.RecoilRangeX.Y),
      0.0
    );
}

FRotator URecoilHandler::Interp(const FRotator & Current, const FRotator & Target, float Alpha)
{
  const float Val = FMath::Clamp(1.0f - FMath::Pow(1.0f - Alpha, 5.0f), 0.0f, 1.0f);
  if (FMath::IsNearlyZero(Val))
    return Current;

  const FRotator Delta = (Target - Current).GetNormalized();
  if (Delta.IsNearlyZero())
    return Target;

  const FRotator DeltaMove = Delta * Val;
  return (Current + DeltaMove).GetNormalized();
}