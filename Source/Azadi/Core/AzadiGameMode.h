// AZADI: Rise of the Dawn

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Data/AzadiTypes.h"
#include "AzadiGameMode.generated.h"

class AAzadiEnemy;

/**
 * Single-player district session: tracks the active objective, enemy
 * counts, rescues and player death/respawn, then hands progression to the
 * campaign subsystem when the district is complete.
 */
UCLASS()
class AZADI_API AAzadiGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AAzadiGameMode();

	virtual void StartPlay() override;

	// --- World event notifications ---
	void RegisterEnemySpawned(AAzadiEnemy* Enemy);
	void NotifyEnemyKilled(AAzadiEnemy* Enemy);
	void NotifyCitizenFreed();
	void NotifyExitReached(APawn* Pawn);
	void NotifyPlayerDied(AController* PlayerController);

	/** District-wide simultaneous enemy budget gate for spawners. */
	bool CanSpawnEnemy() const;

	// --- HUD state ---
	FString GetObjectiveText() const;
	FString GetBannerText() const { return BannerText; }
	float GetBannerEndTime() const { return BannerEndTime; }
	bool IsDistrictComplete() const { return bDistrictComplete; }
	float GetRespawnTime() const { return RespawnTime; }

	int32 GetKillCount() const { return Killed; }
	int32 GetCagesFreed() const { return CagesFreed; }
	int32 GetCagesTotal() const { return CagesTotal; }

protected:
	UFUNCTION()
	void HandleMilestone(float Milestone);

private:
	void CheckObjective();
	void CompleteDistrict();
	void TravelNext();
	void RespawnPlayer(AController* PlayerController);
	void ShowBanner(const FString& Text, float Duration);

	FAzadiDistrictDef District;
	bool bHasDistrict = false;
	bool bDistrictComplete = false;

	int32 AliveEnemies = 0;
	int32 Killed = 0;
	int32 CagesTotal = 0;
	int32 CagesFreed = 0;

	FString BannerText;
	float BannerEndTime = -1.f;
	float RespawnTime = -1.f;

	FTimerHandle TravelTimer;
	FTimerHandle RespawnTimer;
};
