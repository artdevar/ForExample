#include "Utils/Input.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"

FText UInput::GetKeyBindedToAction(APlayerController * Controller, UInputAction * Action)
{
  ensure(Controller->IsLocalController());

  auto InputSubsystem = Controller->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

  const TArray<FKey> Binds = InputSubsystem->QueryKeysMappedToAction(Action);
  return Binds.IsEmpty() ? FText() : Binds[0].GetDisplayName();
}
