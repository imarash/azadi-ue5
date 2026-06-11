// AZADI: Rise of the Dawn

#include "World/AzadiSignActor.h"
#include "Azadi.h"
#include "Components/StaticMeshComponent.h"
#include "Core/AzadiDistrictSubsystem.h"
#include "Core/AzadiLiberationSubsystem.h"
#include "Core/AzadiModuleRegistry.h"
#include "Core/AzadiStyleSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AAzadiSignActor::AAzadiSignActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Panel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Panel"));
	RootComponent = Panel;
	Panel->SetCollisionProfileName(TEXT("BlockAll"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh.Succeeded())
	{
		Panel->SetStaticMesh(PlaneMesh.Object);
	}
}

void AAzadiSignActor::BeginPlay()
{
	Super::BeginPlay();

	if (const UAzadiDistrictSubsystem* District = GetWorld()->GetSubsystem<UAzadiDistrictSubsystem>())
	{
		if (District->HasDistrict())
		{
			FlipThreshold = District->GetDistrict().SignageFlipThreshold;
		}
	}

	ApplyTexture(false);

	if (UAzadiLiberationSubsystem* Liberation = GetWorld()->GetSubsystem<UAzadiLiberationSubsystem>())
	{
		Liberation->OnLiberationChanged.AddDynamic(this, &AAzadiSignActor::HandleLiberationChanged);
	}
}

void AAzadiSignActor::HandleLiberationChanged(float NewValue)
{
	if (!bShowingLiberated && NewValue >= FlipThreshold)
	{
		bShowingLiberated = true;
		ApplyTexture(true);

		if (UAzadiStyleSubsystem* Style = GetWorld()->GetSubsystem<UAzadiStyleSubsystem>())
		{
			Style->PlayStinger(TEXT("sign_swap"));
		}
	}
}

void AAzadiSignActor::ApplyTexture(bool bLiberated)
{
	const UAzadiDistrictSubsystem* District = GetWorld()->GetSubsystem<UAzadiDistrictSubsystem>();
	const FAzadiSignageSetDef* SignageSet = District ? District->GetSignageSet() : nullptr;
	if (!SignageSet)
	{
		return;
	}

	const FAzadiSignageSlotDef* Slot = SignageSet->Slots.FindByPredicate(
		[this](const FAzadiSignageSlotDef& S) { return S.SlotId == SlotId; });
	if (!Slot)
	{
		UE_LOG(LogAzadi, Warning, TEXT("Sign %s: slot '%s' missing from signage set '%s'"),
			*GetName(), *SlotId.ToString(), *SignageSet->Id.ToString());
		return;
	}

	UGameInstance* GameInstance = GetWorld()->GetGameInstance();
	UAzadiModuleRegistry* Registry = GameInstance ? GameInstance->GetSubsystem<UAzadiModuleRegistry>() : nullptr;
	if (!Registry)
	{
		return;
	}

	UTexture2D* Texture = Registry->ResolveTexture(bLiberated ? Slot->LiberatedTexture : Slot->OccupiedTexture);
	if (!Texture)
	{
		return;
	}

	UMaterialInterface* SignMaterial = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/Azadi/Materials/M_AzadiSign.M_AzadiSign"));
	if (!SignMaterial)
	{
		UE_LOG(LogAzadi, Warning, TEXT("Sign %s: M_AzadiSign missing — run the azadi_bootstrap.py editor script"), *GetName());
		return;
	}

	if (UMaterialInstanceDynamic* Mid = Panel->CreateDynamicMaterialInstance(0, SignMaterial))
	{
		Mid->SetTextureParameterValue(TEXT("SignTex"), Texture);
		Mid->SetScalarParameterValue(TEXT("Emissive"), bLiberated ? 1.2f : 0.35f);
	}
}
