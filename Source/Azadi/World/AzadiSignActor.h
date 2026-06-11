// AZADI: Rise of the Dawn

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AzadiSignActor.generated.h"

class UStaticMeshComponent;

/**
 * A signage surface (billboard, awning, mural wall). Pulls its texture from
 * the district's signage set by SlotId: propaganda while occupied, mural
 * once the AZADI meter passes the district's flip threshold.
 *
 * Signage realism is a StylePack-level concern: swap the signage set in
 * data (or via a mod) and every sign in the district changes.
 */
UCLASS()
class AZADI_API AAzadiSignActor : public AActor
{
	GENERATED_BODY()

public:
	AAzadiSignActor();

	/** Which slot of the district's signage set this sign displays. */
	UPROPERTY(EditAnywhere, Category = "Azadi")
	FName SlotId;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleLiberationChanged(float NewValue);

	UPROPERTY(VisibleAnywhere, Category = "Azadi")
	TObjectPtr<UStaticMeshComponent> Panel;

private:
	void ApplyTexture(bool bLiberated);

	bool bShowingLiberated = false;
	float FlipThreshold = 0.5f;
};
