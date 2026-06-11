// AZADI: Rise of the Dawn

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AzadiExitZone.generated.h"

class UBoxComponent;

/** District exit trigger for ReachExit objectives. */
UCLASS()
class AZADI_API AAzadiExitZone : public AActor
{
	GENERATED_BODY()

public:
	AAzadiExitZone();

	virtual void PostInitializeComponents() override;

protected:
	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, Category = "Azadi")
	TObjectPtr<UBoxComponent> Zone;
};
