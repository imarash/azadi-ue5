// AZADI: Rise of the Dawn

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/AzadiTypes.h"
#include "AzadiStyleSubsystem.generated.h"

class UAudioComponent;
class UFont;
class UPostProcessComponent;

/**
 * StylePack module: HUD skin, color grade, and adaptive audio stems —
 * swappable per district without touching code.
 *
 * The grade is applied through an unbound post-process component and lerps
 * toward the pack's "liberated" look as the AZADI meter rises. Music
 * crossfades calm/combat stems and switches to the anthem at full liberation.
 */
UCLASS()
class AZADI_API UAzadiStyleSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override
	{
		return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
	}

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	UFUNCTION(BlueprintCallable, Category = "Azadi")
	void ApplyStylePack(FName StylePackId);

	const FAzadiStylePackDef& GetStyle() const { return Style; }

	/** HUD font: style pack override or the engine fallback. */
	UFont* GetHudFont() const;

	/** Play a named one-shot from the pack's stinger map ("cheer", ...). */
	void PlayStinger(FName StingerId);

private:
	UFUNCTION()
	void HandleLiberationChanged(float NewValue);

	UFUNCTION()
	void HandleCombatStateChanged(bool bInCombat);

	UFUNCTION()
	void HandleMilestone(float Milestone);

	void EnsurePostProcess();
	void ApplyGrade(float Liberation);
	void StartMusic();

	FAzadiStylePackDef Style;
	bool bStyleApplied = false;

	UPROPERTY(Transient)
	TObjectPtr<UPostProcessComponent> PostProcess;

	UPROPERTY(Transient)
	TObjectPtr<UFont> HudFont;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> CalmMusic;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> CombatMusic;
};
