#include "Utils/Input.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"

FText UInput::GetKeyBindedToAction(APlayerController * Controller, UInputAction * Action)
{
  UEnhancedInputLocalPlayerSubsystem * Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Controller->GetLocalPlayer());
  const TArray<FKey> Binds = Subsystem->QueryKeysMappedToAction(Action);

  return Binds.IsEmpty() ? FText() : Binds[0].GetDisplayName();
}
