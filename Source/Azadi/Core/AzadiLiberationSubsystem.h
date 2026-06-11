// AZADI: Rise of the Dawn

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AzadiLiberationSubsystem.generated.h"

class UMaterialParameterCollection;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAzadiLiberationChanged, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAzadiLiberationMilestone, float, Milestone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAzadiCombatStateChanged, bool, bInCombat);

/**
 * Liberation module: the AZADI meter and how it bleeds into the world.
 * Kills, rescues, and objectives feed the meter; materials (via the MPC),
 * signage, music, and the color grade all listen.
 */
UCLASS()
class AZADI_API UAzadiLiberationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override
	{
		return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
	}

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	UFUNCTION(BlueprintCallable, Category = "Azadi")
	void AddLiberation(float Amount);

	UFUNCTION(BlueprintPure, Category = "Azadi")
	float GetLiberation() const { return Meter; }

	UFUNCTION(BlueprintPure, Category = "Azadi")
	bool IsFullyLiberated() const { return Meter >= 1.f; }

	/** Enemies report engagement so music/tension systems know combat state. */
	void ReportCombatant(bool bEngaged);

	UFUNCTION(BlueprintPure, Category = "Azadi")
	bool IsCombatActive() const { return EngagedCount > 0; }

	UPROPERTY(BlueprintAssignable, Category = "Azadi")
	FAzadiLiberationChanged OnLiberationChanged;

	/** Fired once per milestone: 0.25, 0.5, 0.75, 1.0 */
	UPROPERTY(BlueprintAssignable, Category = "Azadi")
	FAzadiLiberationMilestone OnMilestone;

	UPROPERTY(BlueprintAssignable, Category = "Azadi")
	FAzadiCombatStateChanged OnCombatStateChanged;

private:
	void PushToMaterials() const;

	float Meter = 0.f;
	int32 EngagedCount = 0;
	uint8 MilestonesFired = 0;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialParameterCollection> Mpc;
};
