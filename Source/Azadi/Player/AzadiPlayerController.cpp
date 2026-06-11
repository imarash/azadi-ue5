// AZADI: Rise of the Dawn

#include "Player/AzadiPlayerController.h"

void AAzadiPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	SetShowMouseCursor(false);
}
