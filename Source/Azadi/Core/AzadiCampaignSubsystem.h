// AZADI: Rise of the Dawn

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/AzadiTypes.h"
#include "AzadiCampaignSubsystem.generated.h"

class UAzadiModuleRegistry;

/**
 * Campaign module: district order, progression, and level travel.
 * The campaign is an ordered list of district ids (data-driven); any map can
 * also be played directly — the subsystem syncs to whatever world loads.
 */
UCLASS()
class AZADI_API UAzadiCampaignSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** District for a loaded world, matched by map name. Null if the map is not a district. */
	const FAzadiDistrictDef* GetDistrictForWorld(const UWorld* World) const;

	/** Next district id in campaign order, NAME_None if Current is last (or unknown). */
	FName GetNextDistrictId(FName CurrentId) const;

	bool IsFinalDistrict(FName DistrictId) const;

	/** Open the district's map. Returns false if the district is unknown. */
	bool TravelToDistrict(UObject* WorldContextObject, FName DistrictId);

	/** Districts completed this session (campaign progression). */
	UFUNCTION(BlueprintCallable, Category = "Azadi")
	void MarkDistrictCompleted(FName DistrictId);

	UFUNCTION(BlueprintPure, Category = "Azadi")
	bool IsDistrictCompleted(FName DistrictId) const { return CompletedDistricts.Contains(DistrictId); }

private:
	UPROPERTY(Transient)
	TObjectPtr<UAzadiModuleRegistry> Registry;

	TSet<FName> CompletedDistricts;
};
