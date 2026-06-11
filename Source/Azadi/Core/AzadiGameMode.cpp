// AZADI: Rise of the Dawn

#include "Core/AzadiGameMode.h"
#include "AI/AzadiEnemy.h"
#include "Azadi.h"
#include "Core/AzadiCampaignSubsystem.h"
#include "Core/AzadiDistrictSubsystem.h"
#include "Core/AzadiLiberationSubsystem.h"
#include "Core/AzadiStyleSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Player/AzadiCharacter.h"
#include "Player/AzadiHUD.h"
#include "Player/AzadiPlayerController.h"
#include "TimerManager.h"
#include "World/AzadiCitizenCage.h"

AAzadiGameMode::AAzadiGameMode()
{
	DefaultPawnClass = AAzadiCharacter::StaticClass();
	PlayerControllerClass = AAzadiPlayerController::StaticClass();
	HUDClass = AAzadiHUD::StaticClass();
}

void AAzadiGameMode::StartPlay()
{
	Super::StartPlay();

	if (const UAzadiDistrictSubsystem* DistrictSubsystem = GetWorld()->GetSubsystem<UAzadiDistrictSubsystem>())
	{
		bHasDistrict = DistrictSubsystem->HasDistrict();
		if (bHasDistrict)
		{
			District = DistrictSubsystem->GetDistrict();
		}
	}

	for (TActorIterator<AAzadiCitizenCage> It(GetWorld()); It; ++It)
	{
		++CagesTotal;
	}

	if (UAzadiLiberationSubsystem* Liberation = GetWorld()->GetSubsystem<UAzadiLiberationSubsystem>())
	{
		Liberation->OnMilestone.AddDynamic(this, &AAzadiGameMode::HandleMilestone);
	}

	if (bHasDistrict)
	{
		ShowBanner(District.DisplayName.IsEmpty() ? District.Id.ToString().ToUpper() : District.DisplayName.ToUpper(), 4.f);
	}
}

bool AAzadiGameMode::CanSpawnEnemy() const
{
	const int32 Budget = bHasDistrict ? District.EnemyBudget : 8;
	return AliveEnemies < Budget;
}

void AAzadiGameMode::RegisterEnemySpawned(AAzadiEnemy* Enemy)
{
	++AliveEnemies;
}

void AAzadiGameMode::NotifyEnemyKilled(AAzadiEnemy* Enemy)
{
	AliveEnemies = FMath::Max(0, AliveEnemies - 1);
	++Killed;
	CheckObjective();
}

void AAzadiGameMode::NotifyCitizenFreed()
{
	++CagesFreed;
	if (UAzadiStyleSubsystem* Style = GetWorld()->GetSubsystem<UAzadiStyleSubsystem>())
	{
		Style->PlayStinger(TEXT("cheer"));
	}
	CheckObjective();
}

void AAzadiGameMode::NotifyExitReached(APawn* Pawn)
{
	if (!bDistrictComplete && (!bHasDistrict || District.Objective == EAzadiObjective::ReachExit))
	{
		CompleteDistrict();
	}
}

void AAzadiGameMode::HandleMilestone(float Milestone)
{
	if (Milestone >= 1.f && bHasDistrict && District.Objective == EAzadiObjective::Liberate)
	{
		CheckObjective();
	}
}

void AAzadiGameMode::CheckObjective()
{
	if (bDistrictComplete || !bHasDistrict)
	{
		return;
	}

	bool bComplete = false;
	switch (District.Objective)
	{
	case EAzadiObjective::Liberate:
	{
		const UAzadiLiberationSubsystem* Liberation = GetWorld()->GetSubsystem<UAzadiLiberationSubsystem>();
		bComplete = Liberation && Liberation->IsFullyLiberated();
		break;
	}
	case EAzadiObjective::Rescue:
		bComplete = CagesTotal > 0 && CagesFreed >= CagesTotal;
		break;
	case EAzadiObjective::Clear:
		bComplete = Killed >= District.TotalEnemies;
		break;
	case EAzadiObjective::ReachExit:
	default:
		break; // handled by NotifyExitReached
	}

	if (bComplete)
	{
		CompleteDistrict();
	}
}

void AAzadiGameMode::CompleteDistrict()
{
	if (bDistrictComplete)
	{
		return;
	}
	bDistrictComplete = true;

	UAzadiCampaignSubsystem* Campaign = GetGameInstance()->GetSubsystem<UAzadiCampaignSubsystem>();
	if (bHasDistrict && Campaign)
	{
		Campaign->MarkDistrictCompleted(District.Id);
		if (Campaign->IsFinalDistrict(District.Id))
		{
			ShowBanner(TEXT("THE DAWN RISES — CITY LIBERATED"), 12.f);
			return; // campaign over; stay and celebrate
		}
	}

	ShowBanner(TEXT("DISTRICT LIBERATED"), 5.f);
	GetWorld()->GetTimerManager().SetTimer(TravelTimer, this, &AAzadiGameMode::TravelNext, 5.f, false);
}

void AAzadiGameMode::TravelNext()
{
	UAzadiCampaignSubsystem* Campaign = GetGameInstance()->GetSubsystem<UAzadiCampaignSubsystem>();
	if (!Campaign || !bHasDistrict)
	{
		return;
	}
	const FName Next = Campaign->GetNextDistrictId(District.Id);
	if (!Next.IsNone())
	{
		Campaign->TravelToDistrict(this, Next);
	}
}

void AAzadiGameMode::NotifyPlayerDied(AController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}
	UE_LOG(LogAzadi, Log, TEXT("Player fell — respawning in 3s"));
	RespawnTime = GetWorld()->GetTimeSeconds() + 3.f;

	FTimerDelegate Delegate;
	Delegate.BindUObject(this, &AAzadiGameMode::RespawnPlayer, PlayerController);
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, Delegate, 3.f, false);
}

void AAzadiGameMode::RespawnPlayer(AController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}
	if (APawn* OldPawn = PlayerController->GetPawn())
	{
		PlayerController->UnPossess();
		OldPawn->Destroy();
	}
	RespawnTime = -1.f;
	RestartPlayer(PlayerController);
}

void AAzadiGameMode::ShowBanner(const FString& Text, float Duration)
{
	BannerText = Text;
	BannerEndTime = GetWorld()->GetTimeSeconds() + Duration;
}

FString AAzadiGameMode::GetObjectiveText() const
{
	if (!bHasDistrict)
	{
		return TEXT("Explore");
	}
	switch (District.Objective)
	{
	case EAzadiObjective::ReachExit:
		return TEXT("Push through and reach the exit");
	case EAzadiObjective::Liberate:
		return TEXT("Raise AZADI to 100%");
	case EAzadiObjective::Rescue:
		return FString::Printf(TEXT("Free the prisoners  %d / %d"), CagesFreed, CagesTotal);
	case EAzadiObjective::Clear:
		return FString::Printf(TEXT("Break the patrols  %d / %d"), Killed, District.TotalEnemies);
	}
	return FString();
}
