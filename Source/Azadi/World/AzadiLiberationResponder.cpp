// AZADI: Rise of the Dawn

#include "World/AzadiLiberationResponder.h"
#include "Core/AzadiLiberationSubsystem.h"
#include "Engine/World.h"

AAzadiLiberationResponder::AAzadiLiberationResponder()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AAzadiLiberationResponder::BeginPlay()
{
	Super::BeginPlay();

	Apply(false);

	if (UAzadiLiberationSubsystem* Liberation = GetWorld()->GetSubsystem<UAzadiLiberationSubsystem>())
	{
		Liberation->OnLiberationChanged.AddDynamic(this, &AAzadiLiberationResponder::HandleLiberationChanged);
		if (Liberation->GetLiberation() >= Threshold)
		{
			HandleLiberationChanged(Liberation->GetLiberation());
		}
	}
}

void AAzadiLiberationResponder::HandleLiberationChanged(float NewValue)
{
	if (!bApplied && NewValue >= Threshold)
	{
		bApplied = true;
		Apply(true);
	}
}

void AAzadiLiberationResponder::Apply(bool bLiberated)
{
	for (AActor* Actor : ShowWhenLiberated)
	{
		if (Actor)
		{
			Actor->SetActorHiddenInGame(!bLiberated);
			Actor->SetActorEnableCollision(bLiberated);
		}
	}
	for (AActor* Actor : HideWhenLiberated)
	{
		if (Actor)
		{
			Actor->SetActorHiddenInGame(bLiberated);
			Actor->SetActorEnableCollision(!bLiberated);
		}
	}
}
