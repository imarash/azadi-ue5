// AZADI: Rise of the Dawn

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AzadiEnemyController.generated.h"

class AAzadiEnemy;

/**
 * Minimal urban-combat brain: idle until the player is seen, chase via
 * navmesh (with a direct-steering fallback for graybox maps without nav),
 * attack inside the def's range. Behavior numbers come from the enemy def.
 */
UCLASS()
class AZADI_API AAzadiEnemyController : public AAIController
{
	GENERATED_BODY()

public:
	AAzadiEnemyController();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:
	enum class EState : uint8 { Idle, Chase, Attack };

	void Think();
	bool HasLineOfSight(const APawn* Target) const;
	APawn* GetPlayerTarget() const;
	void SetEngaged(bool bNewEngaged);

	EState State = EState::Idle;
	FTimerHandle ThinkTimer;
	float LastRepathTime = 0.f;
	bool bUseDirectSteering = false;
	bool bEngaged = false;
};
