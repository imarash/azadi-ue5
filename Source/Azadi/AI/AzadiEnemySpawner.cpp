// AZADI: Rise of the Dawn

#include "AI/AzadiEnemySpawner.h"
#include "AI/AzadiEnemy.h"
#include "Azadi.h"
#include "Core/AzadiGameMode.h"
#include "Core/AzadiModuleRegistry.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AAzadiEnemySpawner::AAzadiEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AAzadiEnemySpawner::ConfigureFromDistrict(const TArray<FAzadiSpawnEntry>& Table, int32 InMaxAlive, int32 InTotalToSpawn)
{
	DistrictTable = Table;
	MaxAlive = InMaxAlive;
	TotalToSpawn = InTotalToSpawn;
}

void AAzadiEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetTimerManager().SetTimer(SpawnTimer, this, &AAzadiEnemySpawner::TrySpawn,
		FMath::Max(0.25f, SpawnInterval), true, InitialDelay);
}

void AAzadiEnemySpawner::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
	Super::EndPlay(EndPlayReason);
}

void AAzadiEnemySpawner::TrySpawn()
{
	if (GetRemainingToSpawn() <= 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimer);
		return;
	}
	if (AliveCount >= MaxAlive)
	{
		return;
	}

	if (ActivationRadius > 0.f)
	{
		const APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (!Player || FVector::Dist(Player->GetActorLocation(), GetActorLocation()) > ActivationRadius)
		{
			return;
		}
	}

	// Respect the district-wide simultaneous budget.
	AAzadiGameMode* GameMode = GetWorld()->GetAuthGameMode<AAzadiGameMode>();
	if (GameMode && !GameMode->CanSpawnEnemy())
	{
		return;
	}

	const UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	UAzadiModuleRegistry* Registry = GameInstance ? GameInstance->GetSubsystem<UAzadiModuleRegistry>() : nullptr;
	if (!Registry)
	{
		return;
	}

	const FName EnemyId = PickEnemyId();
	const FAzadiEnemyDef* Def = Registry->FindEnemy(EnemyId);
	if (!Def)
	{
		UE_LOG(LogAzadi, Warning, TEXT("Spawner %s: unknown enemy id '%s'"), *GetName(), *EnemyId.ToString());
		return;
	}

	const FVector Offset(FMath::FRandRange(-120.f, 120.f), FMath::FRandRange(-120.f, 120.f), 0.f);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AAzadiEnemy* Enemy = GetWorld()->SpawnActor<AAzadiEnemy>(AAzadiEnemy::StaticClass(),
		GetActorLocation() + Offset + FVector(0.f, 0.f, 100.f), GetActorRotation(), SpawnParams);
	if (!Enemy)
	{
		return;
	}

	Enemy->InitFromDef(*Def);
	Enemy->OnEnemyDied.AddDynamic(this, &AAzadiEnemySpawner::HandleEnemyDied);
	++SpawnedCount;
	++AliveCount;
	UE_LOG(LogAzadi, Log, TEXT("Spawner %s: %s up (%d/%d spawned)"),
		*GetName(), *EnemyId.ToString(), SpawnedCount, TotalToSpawn);
}

FName AAzadiEnemySpawner::PickEnemyId() const
{
	const TArray<FAzadiSpawnEntry>& Table = AuthoredEntries.Num() > 0 ? AuthoredEntries : DistrictTable;
	if (Table.IsEmpty())
	{
		return TEXT("wraith");
	}

	float TotalWeight = 0.f;
	for (const FAzadiSpawnEntry& Entry : Table)
	{
		TotalWeight += FMath::Max(0.f, Entry.Weight);
	}
	float Roll = FMath::FRandRange(0.f, TotalWeight);
	for (const FAzadiSpawnEntry& Entry : Table)
	{
		Roll -= FMath::Max(0.f, Entry.Weight);
		if (Roll <= 0.f)
		{
			return Entry.EnemyId;
		}
	}
	return Table.Last().EnemyId;
}

void AAzadiEnemySpawner::HandleEnemyDied(AAzadiEnemy* Enemy)
{
	AliveCount = FMath::Max(0, AliveCount - 1);
}
