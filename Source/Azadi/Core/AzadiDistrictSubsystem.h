// AZADI: Rise of the Dawn

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/AzadiTypes.h"
#include "AzadiDistrictSubsystem.generated.h"

/**
 * District module: applies the active district's data to the loaded world —
 * spawner budgets, lighting preset, style pack, signage set. Per-level logic
 * without per-level code.
 */
UCLASS()
class AZADI_API UAzadiDistrictSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override
	{
		return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
	}

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	bool HasDistrict() const { return bHasDistrict; }
	const FAzadiDistrictDef& GetDistrict() const { return District; }

	/** Resolved signage set for this district (null when none). */
	const FAzadiSignageSetDef* GetSignageSet() const;

private:
	void ApplyLightingPreset(UWorld& World) const;
	void ConfigureSpawners(UWorld& World) const;

	FAzadiDistrictDef District;
	bool bHasDistrict = false;
};
