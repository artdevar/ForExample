#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Input.generated.h"

class APlayerController;
class UInputAction;

UCLASS()
class FOREXAMPLE_API UInput : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()

public:

  static FText GetKeyBindedToAction(APlayerController * Controller, UInputAction * Action);

};
