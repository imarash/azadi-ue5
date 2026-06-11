// AZADI: Rise of the Dawn

#include "World/AzadiExitZone.h"
#include "Components/BoxComponent.h"
#include "Core/AzadiGameMode.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

AAzadiExitZone::AAzadiExitZone()
{
	PrimaryActorTick.bCanEverTick = false;

	Zone = CreateDefaultSubobject<UBoxComponent>(TEXT("Zone"));
	RootComponent = Zone;
	Zone->SetBoxExtent(FVector(300.f, 300.f, 200.f));
	Zone->SetCollisionProfileName(TEXT("Trigger"));
}

void AAzadiExitZone::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (OtherActor && OtherActor == PlayerPawn)
	{
		if (AAzadiGameMode* GameMode = GetWorld()->GetAuthGameMode<AAzadiGameMode>())
		{
			GameMode->NotifyExitReached(Cast<APawn>(OtherActor));
		}
	}
}

void AAzadiExitZone::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	Zone->OnComponentBeginOverlap.AddDynamic(this, &AAzadiExitZone::HandleOverlap);
}
