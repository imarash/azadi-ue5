// AZADI: Rise of the Dawn

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AzadiCitizenCage.generated.h"

class UStaticMeshComponent;

/**
 * Citizen rescue stub: a checkpoint holding cell. Interact (E) to free the
 * citizens — bumps the AZADI meter and counts toward Rescue objectives.
 */
UCLASS()
class AZADI_API AAzadiCitizenCage : public AActor
{
	GENERATED_BODY()

public:
	AAzadiCitizenCage();

	/** Free the citizens. Returns true the first time. */
	bool TryFree(APawn* Liberator);

	bool IsFreed() const { return bFreed; }

	FString GetPromptText() const;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Azadi")
	TObjectPtr<UStaticMeshComponent> Frame;

	UPROPERTY(VisibleAnywhere, Category = "Azadi")
	TObjectPtr<UStaticMeshComponent> Bars;

private:
	bool bFreed = false;
};
