// AZADI: Rise of the Dawn

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AzadiLiberationResponder.generated.h"

/**
 * Generic "world reacts to hope" hook: shows/hides target actors when the
 * AZADI meter crosses a threshold (string lights, celebration props,
 * regime barriers melting away). Place in a level, point at actors, done.
 */
UCLASS()
class AZADI_API AAzadiLiberationResponder : public AActor
{
	GENERATED_BODY()

public:
	AAzadiLiberationResponder();

	UPROPERTY(EditAnywhere, Category = "Azadi")
	float Threshold = 0.5f;

	/** Hidden until the threshold, then shown (celebration props). */
	UPROPERTY(EditAnywhere, Category = "Azadi")
	TArray<TObjectPtr<AActor>> ShowWhenLiberated;

	/** Visible until the threshold, then hidden (regime props). */
	UPROPERTY(EditAnywhere, Category = "Azadi")
	TArray<TObjectPtr<AActor>> HideWhenLiberated;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleLiberationChanged(float NewValue);

private:
	void Apply(bool bLiberated);

	bool bApplied = false;
};
