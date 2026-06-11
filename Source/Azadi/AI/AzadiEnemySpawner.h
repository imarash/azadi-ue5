// AZADI: Rise of the Dawn

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/AzadiTypes.h"
#include "AzadiEnemySpawner.generated.h"

class AAzadiEnemy;

/**
 * Placeable spawn point. The district subsystem hands each spawner a share
 * of the district's enemy budget; designers can also hand-author entries
 * per spawner (authored entries win over the district table).
 */
UCLASS()
class AZADI_API AAzadiEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AAzadiEnemySpawner();

	/** Called by the district subsystem to distribute the district budget. */
	void ConfigureFromDistrict(const TArray<FAzadiSpawnEntry>& Table, int32 InMaxAlive, int32 InTotalToSpawn);

	/** Enemies this spawner still owes the district. */
	int32 GetRemainingToSpawn() const { return FMath::Max(0, TotalToSpawn - SpawnedCount); }

	int32 GetAliveCount() const { return AliveCount; }

	/** Hand-authored override; empty = use the district spawn table. */
	UPROPERTY(EditAnywhere, Category = "Azadi")
	TArray<FAzadiSpawnEntry> AuthoredEntries;

	UPROPERTY(EditAnywhere, Category = "Azadi")
	int32 MaxAlive = 2;

	UPROPERTY(EditAnywhere, Category = "Azadi")
	int32 TotalToSpawn = 4;

	UPROPERTY(EditAnywhere, Category = "Azadi")
	float InitialDelay = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Azadi")
	float SpawnInterval = 2.5f;

	/** Only spawn while the player is within this range (0 = always). */
	UPROPERTY(EditAnywhere, Category = "Azadi")
	float ActivationRadius = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

private:
	void TrySpawn();
	FName PickEnemyId() const;

	UFUNCTION()
	void HandleEnemyDied(AAzadiEnemy* Enemy);

	TArray<FAzadiSpawnEntry> DistrictTable;
	FTimerHandle SpawnTimer;
	int32 SpawnedCount = 0;
	int32 AliveCount = 0;
};
