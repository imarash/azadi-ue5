// AZADI: Rise of the Dawn

#include "Core/AzadiCampaignSubsystem.h"
#include "Azadi.h"
#include "Core/AzadiModuleRegistry.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UAzadiCampaignSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Registry = Collection.InitializeDependency<UAzadiModuleRegistry>();
}

const FAzadiDistrictDef* UAzadiCampaignSubsystem::GetDistrictForWorld(const UWorld* World) const
{
	if (!World || !Registry)
	{
		return nullptr;
	}
	const FString ShortName = UWorld::RemovePIEPrefix(World->GetMapName());
	return Registry->FindDistrictByMap(ShortName);
}

FName UAzadiCampaignSubsystem::GetNextDistrictId(FName CurrentId) const
{
	if (!Registry)
	{
		return NAME_None;
	}
	const TArray<FName>& Order = Registry->GetCampaign().Districts;
	const int32 Index = Order.IndexOfByKey(CurrentId);
	if (Index == INDEX_NONE || Index + 1 >= Order.Num())
	{
		return NAME_None;
	}
	return Order[Index + 1];
}

bool UAzadiCampaignSubsystem::IsFinalDistrict(FName DistrictId) const
{
	if (!Registry)
	{
		return false;
	}
	const TArray<FName>& Order = Registry->GetCampaign().Districts;
	return Order.Num() > 0 && Order.Last() == DistrictId;
}

bool UAzadiCampaignSubsystem::TravelToDistrict(UObject* WorldContextObject, FName DistrictId)
{
	const FAzadiDistrictDef* District = Registry ? Registry->FindDistrict(DistrictId) : nullptr;
	if (!District || District->Map.IsEmpty())
	{
		UE_LOG(LogAzadi, Warning, TEXT("Campaign: cannot travel to unknown district '%s'"), *DistrictId.ToString());
		return false;
	}
	const FString MapPath = District->MapRoot / District->Map;
	UE_LOG(LogAzadi, Log, TEXT("Campaign: traveling to %s (%s)"), *DistrictId.ToString(), *MapPath);
	UGameplayStatics::OpenLevel(WorldContextObject, FName(*MapPath));
	return true;
}

void UAzadiCampaignSubsystem::MarkDistrictCompleted(FName DistrictId)
{
	CompletedDistricts.Add(DistrictId);
	UE_LOG(LogAzadi, Log, TEXT("Campaign: district '%s' liberated (%d total)"),
		*DistrictId.ToString(), CompletedDistricts.Num());
}
