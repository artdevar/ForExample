#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Shared/Weapons.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "WeaponData.generated.h"

USTRUCT(BlueprintType)
struct FOREXAMPLE_API FWeaponData : public FTableRowBase
{
  GENERATED_BODY()

public:

  UPROPERTY(EditDefaultsOnly)
  TMap<EWeaponSound, FSoundsSet> SoundsSet;

  UPROPERTY(EditDefaultsOnly)
  TMap<TSoftObjectPtr<UPhysicalMaterial>, int32> HitboxDamage;

  UPROPERTY(EditDefaultsOnly)
  int32 MagazineAmount;

  UPROPERTY(EditDefaultsOnly)
  float FireRate;

  UPROPERTY(EditDefaultsOnly)
  FWeaponRecoilParams RecoilParams;

  UPROPERTY(EditDefaultsOnly)
  TArray<EWeaponFireMode> FireModes;
};