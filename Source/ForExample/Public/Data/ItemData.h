#pragma once

#include "CoreMinimal.h"
#include "ItemData.generated.h"

USTRUCT(BlueprintType)
struct FItemData
{
  GENERATED_BODY()

public:

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
  FString ItemName;

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
  int32 ItemID;

};