#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Weapons.generated.h"

USTRUCT(BlueprintType)
struct FWeaponRecoilParams
{
  GENERATED_BODY()

public:

  UPROPERTY(EditDefaultsOnly)
  float RecoilSmoothness = 1.0f;

  UPROPERTY(EditDefaultsOnly)
  float RecoilStopSpeed = 1.0f;

  UPROPERTY(EditDefaultsOnly)
  FVector2f RecoilRangeX;

  UPROPERTY(EditDefaultsOnly)
  FVector2f RecoilRangeY;
};

UENUM(BlueprintType)
enum class EWeaponSound : uint8
{
  Shoot          UMETA(DisplayName="Shoot"),
  NoAmmo         UMETA(DisplayName="NoAmmo"),
  Reload_1       UMETA(DisplayName="Reload_1"),
  Reload_2       UMETA(DisplayName="Reload_2"),
  Reload_3       UMETA(DisplayName="Reload_3"),
  FireModeSwitch UMETA(DisplayName="FireModeSwitch")
};

UENUM(BlueprintType)
enum class EWeaponFireMode : uint8
{
  Single    UMETA(DisplayName="Single"),
  Burst     UMETA(DisplayName="Burst"),
  Automatic UMETA(DisplayName="Automatic")
};

UCLASS()
class FOREXAMPLE_API UWeapons : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:



};
