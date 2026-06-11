// AZADI: Rise of the Dawn

#include "World/AzadiCitizenCage.h"
#include "Components/StaticMeshComponent.h"
#include "Core/AzadiDistrictSubsystem.h"
#include "Core/AzadiGameMode.h"
#include "Core/AzadiLiberationSubsystem.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

AAzadiCitizenCage::AAzadiCitizenCage()
{
	PrimaryActorTick.bCanEverTick = false;

	Frame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Frame"));
	RootComponent = Frame;
	Frame->SetCollisionProfileName(TEXT("BlockAll"));
	Frame->SetRelativeScale3D(FVector(2.2f, 2.2f, 0.12f));

	Bars = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Bars"));
	Bars->SetupAttachment(Frame);
	Bars->SetCollisionProfileName(TEXT("BlockAll"));
	// Counter the frame's flat scale to make a tall thin cage box.
	Bars->SetRelativeScale3D(FVector(0.95f, 0.95f, 18.f));
	Bars->SetRelativeLocation(FVector(0.f, 0.f, 9.f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Frame->SetStaticMesh(CubeMesh.Object);
		Bars->SetStaticMesh(CubeMesh.Object);
	}
}

bool AAzadiCitizenCage::TryFree(APawn* Liberator)
{
	if (bFreed)
	{
		return false;
	}
	bFreed = true;

	Bars->SetVisibility(false);
	Bars->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	float Reward = 0.15f;
	if (const UAzadiDistrictSubsystem* District = GetWorld()->GetSubsystem<UAzadiDistrictSubsystem>())
	{
		if (District->HasDistrict())
		{
			Reward = District->GetDistrict().RescueReward;
		}
	}
	if (UAzadiLiberationSubsystem* Liberation = GetWorld()->GetSubsystem<UAzadiLiberationSubsystem>())
	{
		Liberation->AddLiberation(Reward);
	}
	if (AAzadiGameMode* GameMode = GetWorld()->GetAuthGameMode<AAzadiGameMode>())
	{
		GameMode->NotifyCitizenFreed();
	}
	return true;
}

FString AAzadiCitizenCage::GetPromptText() const
{
	return bFreed ? FString() : TEXT("[E]  Free them");
}
