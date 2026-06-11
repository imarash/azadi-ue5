// AZADI: Rise of the Dawn

#include "Player/AzadiHUD.h"
#include "Combat/AzadiHealthComponent.h"
#include "Combat/AzadiWeaponComponent.h"
#include "Core/AzadiGameMode.h"
#include "Core/AzadiLiberationSubsystem.h"
#include "Core/AzadiStyleSubsystem.h"
#include "Data/AzadiTypes.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Engine/World.h"
#include "Player/AzadiCharacter.h"
#include "World/AzadiCitizenCage.h"

void AAzadiHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas)
	{
		return;
	}

	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;
	const float Now = GetWorld()->GetTimeSeconds();
	const float Delta = LastDrawTime > 0.f ? FMath::Min(Now - LastDrawTime, 0.1f) : 0.f;
	LastDrawTime = Now;

	const UAzadiStyleSubsystem* StyleSubsystem = GetWorld()->GetSubsystem<UAzadiStyleSubsystem>();
	static const FAzadiStylePackDef DefaultStyle;
	const FAzadiStylePackDef& Style = StyleSubsystem ? StyleSubsystem->GetStyle() : DefaultStyle;
	UFont* Font = StyleSubsystem ? StyleSubsystem->GetHudFont() : GEngine->GetMediumFont();
	UFont* BigFont = GEngine->GetLargeFont();

	const AAzadiGameMode* GameMode = GetWorld()->GetAuthGameMode<AAzadiGameMode>();
	const AAzadiCharacter* Player = Cast<AAzadiCharacter>(GetOwningPawn());

	// --- AZADI meter (top center) ---
	if (const UAzadiLiberationSubsystem* Liberation = GetWorld()->GetSubsystem<UAzadiLiberationSubsystem>())
	{
		DisplayedMeter = FMath::FInterpTo(DisplayedMeter, Liberation->GetLiberation(), Delta, 4.f);
		DrawMeter(DisplayedMeter, Style);
	}

	// --- Objective (top left) ---
	if (GameMode)
	{
		DrawText(GameMode->GetObjectiveText(), Style.HudText, W * 0.02f, H * 0.03f, Font, 1.1f);
	}

	if (Player)
	{
		const UAzadiWeaponComponent* Weapon = Player->GetWeapon();
		const UAzadiHealthComponent* Health = Player->GetHealth();

		// --- Crosshair / hitmarker ---
		if (!Health->IsDead())
		{
			DrawCrosshair(*Player, Style.CrosshairColor);

			const float SinceHit = Now - Weapon->GetLastHitTime();
			if (SinceHit >= 0.f && SinceHit < 0.18f)
			{
				const float A = 1.f - SinceHit / 0.18f;
				const FLinearColor MarkColor(1.f, 1.f, 1.f, A);
				const float R = 9.f, G = 4.f;
				DrawLine(W * 0.5f - R, H * 0.5f - R, W * 0.5f - G, H * 0.5f - G, MarkColor, 1.5f);
				DrawLine(W * 0.5f + R, H * 0.5f - R, W * 0.5f + G, H * 0.5f - G, MarkColor, 1.5f);
				DrawLine(W * 0.5f - R, H * 0.5f + R, W * 0.5f - G, H * 0.5f + G, MarkColor, 1.5f);
				DrawLine(W * 0.5f + R, H * 0.5f + R, W * 0.5f + G, H * 0.5f + G, MarkColor, 1.5f);
			}
		}

		// --- Health (bottom left) ---
		{
			const float BarW = W * 0.16f, BarH = 10.f;
			const float X = W * 0.02f, Y = H * 0.93f;
			const float Frac = Health->GetHealthFraction();
			DrawRect(Style.HudBackdrop, X - 4.f, Y - 4.f, BarW + 8.f, BarH + 26.f);
			DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.5f), X, Y, BarW, BarH);
			DrawRect(Frac > 0.3f ? Style.HudAccent : Style.DangerColor, X, Y, BarW * Frac, BarH);
			DrawText(FString::Printf(TEXT("%d"), FMath::CeilToInt(Health->GetHealth())),
				Style.HudText, X, Y + BarH + 2.f, Font, 1.f);
		}

		// --- Ammo (bottom right) ---
		{
			const FString Ammo = Weapon->IsReloading() ? TEXT("RELOADING") : Weapon->GetAmmoText();
			const FString Name = Weapon->GetWeaponName();
			float TW, TH;
			GetTextSize(Ammo, TW, TH, BigFont, 1.4f);
			DrawRect(Style.HudBackdrop, W * 0.98f - 180.f, H * 0.93f - 6.f, 184.f, 52.f);
			DrawText(Ammo, Style.HudText, W * 0.98f - TW, H * 0.93f, BigFont, 1.4f);
			float NW, NH;
			GetTextSize(Name, NW, NH, Font, 1.f);
			DrawText(Name, Style.HudAccent, W * 0.98f - NW, H * 0.93f + TH + 2.f, Font, 1.f);
		}

		// --- Damage flash ---
		const float SinceDamage = Now - Health->GetLastDamageTime();
		if (SinceDamage >= 0.f && SinceDamage < 0.35f && !Health->IsDead())
		{
			const float A = (1.f - SinceDamage / 0.35f) * 0.35f;
			const FLinearColor Flash(Style.DangerColor.R, Style.DangerColor.G, Style.DangerColor.B, A);
			const float Edge = H * 0.06f;
			DrawRect(Flash, 0, 0, W, Edge);
			DrawRect(Flash, 0, H - Edge, W, Edge);
			DrawRect(Flash, 0, Edge, Edge, H - 2.f * Edge);
			DrawRect(Flash, W - Edge, Edge, Edge, H - 2.f * Edge);
		}

		// --- Interact prompt ---
		if (!Health->IsDead())
		{
			if (const AAzadiCitizenCage* Cage = Player->FindInteractable())
			{
				DrawTextAligned(Cage->GetPromptText(), W * 0.5f, H * 0.62f, Style.HudAccent, Font, 1.2f);
			}
		}

		// --- Death overlay ---
		if (Health->IsDead())
		{
			DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.55f), 0, 0, W, H);
			DrawTextAligned(TEXT("YOU FELL — THE DAWN STILL RISES"), W * 0.5f, H * 0.42f, Style.DangerColor, BigFont, 1.6f);
			if (GameMode && GameMode->GetRespawnTime() > Now)
			{
				DrawTextAligned(FString::Printf(TEXT("Rising again in %.0f..."), GameMode->GetRespawnTime() - Now),
					W * 0.5f, H * 0.5f, Style.HudText, Font, 1.2f);
			}
		}
	}

	// --- Banner ---
	if (GameMode && GameMode->GetBannerEndTime() > Now)
	{
		const float Remaining = GameMode->GetBannerEndTime() - Now;
		const float A = FMath::Clamp(Remaining, 0.f, 1.f);
		FLinearColor BannerColor = Style.HudAccent;
		BannerColor.A = A;
		DrawTextAligned(GameMode->GetBannerText(), W * 0.5f, H * 0.3f, BannerColor, BigFont, 2.2f);
	}
}

void AAzadiHUD::DrawCrosshair(const AAzadiCharacter& Player, const FLinearColor& Color)
{
	const float W = Canvas->SizeX, H = Canvas->SizeY;
	const float CX = W * 0.5f, CY = H * 0.5f;
	const UAzadiWeaponComponent* Weapon = Player.GetWeapon();

	// Map spread degrees to pixels (approx: vertical FOV reference).
	const float Gap = 6.f + Weapon->GetCurrentSpreadDeg() * (H / 60.f);
	const float Len = 7.f;
	const float Ads = Weapon->GetAdsAlpha();
	const float Alpha = FMath::Lerp(0.9f, 0.5f, Ads);
	FLinearColor C = Color;
	C.A = Alpha;

	DrawRect(C, CX - 1.f, CY - 1.f, 2.f, 2.f);
	DrawLine(CX - Gap - Len, CY, CX - Gap, CY, C, 1.6f);
	DrawLine(CX + Gap, CY, CX + Gap + Len, CY, C, 1.6f);
	DrawLine(CX, CY - Gap - Len, CX, CY - Gap, C, 1.6f);
	DrawLine(CX, CY + Gap, CX, CY + Gap + Len, C, 1.6f);
}

void AAzadiHUD::DrawMeter(float Liberation, const FAzadiStylePackDef& Style)
{
	const float W = Canvas->SizeX, H = Canvas->SizeY;
	const float BarW = W * 0.3f, BarH = 8.f;
	const float X = (W - BarW) * 0.5f, Y = H * 0.035f;

	DrawRect(Style.HudBackdrop, X - 6.f, Y - 22.f, BarW + 12.f, BarH + 30.f);
	DrawTextAligned(TEXT("A Z A D I"), W * 0.5f, Y - 19.f, Style.HudAccent,
		GEngine->GetSmallFont(), 1.f);

	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.5f), X, Y, BarW, BarH);
	DrawRect(Style.HudAccent, X, Y, BarW * FMath::Clamp(Liberation, 0.f, 1.f), BarH);

	for (float Tick : { 0.25f, 0.5f, 0.75f })
	{
		DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.7f), X + BarW * Tick - 1.f, Y, 2.f, BarH);
	}
}

void AAzadiHUD::DrawTextAligned(const FString& Text, float CenterX, float Y, const FLinearColor& Color,
	UFont* Font, float Scale)
{
	float TW, TH;
	GetTextSize(Text, TW, TH, Font, Scale);
	DrawText(Text, Color, CenterX - TW * 0.5f, Y, Font, Scale);
}
