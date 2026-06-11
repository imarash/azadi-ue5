// AZADI: Rise of the Dawn

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AzadiHUD.generated.h"

class AAzadiCharacter;

/**
 * Code-drawn HUD skinned by the active StylePack: crosshair with live
 * spread, AZADI meter, health, ammo, objective, banners, hit markers.
 * UMG/RTL Farsi skins replace this per-StylePack later; the data contract
 * (style colors/fonts) already lives in FAzadiStylePackDef.
 */
UCLASS()
class AZADI_API AAzadiHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawCrosshair(const AAzadiCharacter& Player, const FLinearColor& Color);
	void DrawMeter(float Liberation, const struct FAzadiStylePackDef& Style);
	void DrawTextAligned(const FString& Text, float CenterX, float Y, const FLinearColor& Color,
		UFont* Font, float Scale);

	float DisplayedMeter = 0.f;
	float LastDrawTime = -1.f;
};
