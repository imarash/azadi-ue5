// AZADI: Rise of the Dawn

#include "AI/AzadiEnemyController.h"
#include "AI/AzadiEnemy.h"
#include "Azadi.h"
#include "Combat/AzadiHealthComponent.h"
#include "Core/AzadiLiberationSubsystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "TimerManager.h"

AAzadiEnemyController::AAzadiEnemyController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AAzadiEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	GetWorld()->GetTimerManager().SetTimer(ThinkTimer, this, &AAzadiEnemyController::Think, 0.25f, true,
		FMath::FRandRange(0.f, 0.25f));
}

void AAzadiEnemyController::OnUnPossess()
{
	GetWorld()->GetTimerManager().ClearTimer(ThinkTimer);
	SetEngaged(false);
	Super::OnUnPossess();
}

void AAzadiEnemyController::Think()
{
	AAzadiEnemy* Enemy = Cast<AAzadiEnemy>(GetPawn());
	if (!Enemy || Enemy->IsDead())
	{
		State = EState::Idle;
		SetEngaged(false);
		return;
	}

	APawn* Target = GetPlayerTarget();
	if (!Target)
	{
		State = EState::Idle;
		SetEngaged(false);
		StopMovement();
		return;
	}

	const FAzadiEnemyDef& Def = Enemy->GetDef();
	const float Distance = FVector::Dist(Enemy->GetActorLocation(), Target->GetActorLocation());
	const bool bSeen = Distance <= Def.SightRadius && HasLineOfSight(Target);

	switch (State)
	{
	case EState::Idle:
		if (bSeen)
		{
			State = EState::Chase;
			SetEngaged(true);
		}
		break;

	case EState::Chase:
	{
		if (Distance <= Def.AttackRange && bSeen)
		{
			State = EState::Attack;
			StopMovement();
			break;
		}

		const float Now = GetWorld()->GetTimeSeconds();
		if (Now - LastRepathTime > 0.5f)
		{
			LastRepathTime = Now;
			const EPathFollowingRequestResult::Type Result =
				MoveToActor(Target, FMath::Max(60.f, Def.AttackRange * 0.7f), true);
			// Graybox maps may not have a navmesh yet — steer directly.
			bUseDirectSteering = (Result == EPathFollowingRequestResult::Failed);
		}
		break;
	}

	case EState::Attack:
		if (Distance > Def.AttackRange * 1.15f || !bSeen)
		{
			State = EState::Chase;
			break;
		}
		SetFocus(Target);
		Enemy->TryAttack(Target);
		break;
	}
}

void AAzadiEnemyController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (State == EState::Chase && bUseDirectSteering)
	{
		APawn* Enemy = GetPawn();
		APawn* Target = GetPlayerTarget();
		if (Enemy && Target)
		{
			const FVector Dir = (Target->GetActorLocation() - Enemy->GetActorLocation()).GetSafeNormal2D();
			Enemy->AddMovementInput(Dir, 1.f);
		}
	}
}

bool AAzadiEnemyController::HasLineOfSight(const APawn* Target) const
{
	const APawn* Enemy = GetPawn();
	if (!Enemy || !Target)
	{
		return false;
	}
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AzadiEnemyLOS), false, Enemy);
	const FVector From = Enemy->GetActorLocation() + FVector(0.f, 0.f, 60.f);
	const FVector To = Target->GetActorLocation() + FVector(0.f, 0.f, 40.f);

	FHitResult Hit;
	if (!GetWorld()->LineTraceSingleByChannel(Hit, From, To, ECC_Pawn, Params))
	{
		return true;
	}
	return Hit.GetActor() == Target;
}

APawn* AAzadiEnemyController::GetPlayerTarget() const
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Player)
	{
		return nullptr;
	}
	if (const UAzadiHealthComponent* Health = Player->FindComponentByClass<UAzadiHealthComponent>())
	{
		if (Health->IsDead())
		{
			return nullptr;
		}
	}
	return Player;
}

void AAzadiEnemyController::SetEngaged(bool bNewEngaged)
{
	if (bEngaged == bNewEngaged)
	{
		return;
	}
	bEngaged = bNewEngaged;
	if (UAzadiLiberationSubsystem* Liberation = GetWorld()->GetSubsystem<UAzadiLiberationSubsystem>())
	{
		Liberation->ReportCombatant(bEngaged);
	}
}
