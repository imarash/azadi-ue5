// AZADI: Rise of the Dawn

#include "Core/AzadiDistrictSubsystem.h"
#include "AI/AzadiEnemySpawner.h"
#include "Azadi.h"
#include "Components/DirectionalLightComponent.h"
#include "Core/AzadiCampaignSubsystem.h"
#include "Core/AzadiModuleRegistry.h"
#include "Core/AzadiStyleSubsystem.h"
#include "Engine/DirectionalLight.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"

void UAzadiDistrictSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	const UGameInstance* GameInstance = InWorld.GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	const UAzadiCampaignSubsystem* Campaign = GameInstance->GetSubsystem<UAzadiCampaignSubsystem>();
	const FAzadiDistrictDef* Found = Campaign ? Campaign->GetDistrictForWorld(&InWorld) : nullptr;
	if (!Found)
	{
		UE_LOG(LogAzadi, Log, TEXT("District: no district registered for map '%s'"),
			*UWorld::RemovePIEPrefix(InWorld.GetMapName()));
		return;
	}

	District = *Found;
	bHasDistrict = true;
	UE_LOG(LogAzadi, Log, TEXT("District: applying '%s' (style=%s, signage=%s, budget=%d/%d)"),
		*District.Id.ToString(), *District.StylePackId.ToString(), *District.SignageSetId.ToString(),
		District.EnemyBudget, District.TotalEnemies);

	ApplyLightingPreset(InWorld);
	ConfigureSpawners(InWorld);

	if (UAzadiStyleSubsystem* Style = InWorld.GetSubsystem<UAzadiStyleSubsystem>())
	{
		Style->ApplyStylePack(District.StylePackId);
	}
}

const FAzadiSignageSetDef* UAzadiDistrictSubsystem::GetSignageSet() const
{
	if (!bHasDistrict || District.SignageSetId.IsNone())
	{
		return nullptr;
	}
	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UAzadiModuleRegistry* Registry = GameInstance ? GameInstance->GetSubsystem<UAzadiModuleRegistry>() : nullptr;
	return Registry ? Registry->FindSignageSet(District.SignageSetId) : nullptr;
}

void UAzadiDistrictSubsystem::ApplyLightingPreset(UWorld& World) const
{
	// Presets keep graybox maps readable; finished maps light by hand.
	FRotator SunRotation(-35.f, 40.f, 0.f);
	FLinearColor SunColor(1.f, 0.85f, 0.7f);
	float SunIntensity = 6.f;

	if (District.LightingPreset == TEXT("dusk"))
	{
		SunRotation = FRotator(-12.f, 55.f, 0.f);
		SunColor = FLinearColor(1.f, 0.55f, 0.3f);
		SunIntensity = 4.f;
	}
	else if (District.LightingPreset == TEXT("night"))
	{
		SunRotation = FRotator(-28.f, 210.f, 0.f);
		SunColor = FLinearColor(0.35f, 0.45f, 0.8f);
		SunIntensity = 0.6f;
	}
	else if (District.LightingPreset == TEXT("noon"))
	{
		SunRotation = FRotator(-65.f, 25.f, 0.f);
		SunColor = FLinearColor(1.f, 0.97f, 0.9f);
		SunIntensity = 10.f;
	}

	for (TActorIterator<ADirectionalLight> It(&World); It; ++It)
	{
		if (!It->ActorHasTag(TEXT("AzadiSun")))
		{
			continue;
		}
		It->SetActorRotation(SunRotation);
		if (UDirectionalLightComponent* Light = Cast<UDirectionalLightComponent>(It->GetLightComponent()))
		{
			Light->SetLightColor(SunColor);
			Light->SetIntensity(SunIntensity);
		}
	}
}

void UAzadiDistrictSubsystem::ConfigureSpawners(UWorld& World) const
{
	TArray<AAzadiEnemySpawner*> Spawners;
	for (TActorIterator<AAzadiEnemySpawner> It(&World); It; ++It)
	{
		Spawners.Add(*It);
	}
	if (Spawners.IsEmpty() || District.SpawnTable.IsEmpty())
	{
		return;
	}

	// Deterministic distribution of the district budget across spawners.
	Spawners.Sort([](const AAzadiEnemySpawner& A, const AAzadiEnemySpawner& B)
	{
		return A.GetName() < B.GetName();
	});

	const int32 Count = Spawners.Num();
	const int32 AliveBase = District.EnemyBudget / Count;
	const int32 AliveRemainder = District.EnemyBudget % Count;
	const int32 TotalBase = District.TotalEnemies / Count;
	const int32 TotalRemainder = District.TotalEnemies % Count;

	for (int32 i = 0; i < Count; ++i)
	{
		const int32 MaxAlive = AliveBase + (i < AliveRemainder ? 1 : 0);
		const int32 Total = TotalBase + (i < TotalRemainder ? 1 : 0);
		Spawners[i]->ConfigureFromDistrict(District.SpawnTable, MaxAlive, Total);
	}
}
