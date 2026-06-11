// AZADI: Rise of the Dawn

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AzadiPlayerController.generated.h"

UCLASS()
class AZADI_API AAzadiPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
