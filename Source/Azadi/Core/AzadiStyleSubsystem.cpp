// AZADI: Rise of the Dawn

#include "Core/AzadiStyleSubsystem.h"
#include "Azadi.h"
#include "Components/AudioComponent.h"
#include "Components/PostProcessComponent.h"
#include "Core/AzadiLiberationSubsystem.h"
#include "Core/AzadiModuleRegistry.h"
#include "Engine/Font.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void UAzadiStyleSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (UAzadiLiberationSubsystem* Liberation = InWorld.GetSubsystem<UAzadiLiberationSubsystem>())
	{
		Liberation->OnLiberationChanged.AddDynamic(this, &UAzadiStyleSubsystem::HandleLiberationChanged);
		Liberation->OnCombatStateChanged.AddDynamic(this, &UAzadiStyleSubsystem::HandleCombatStateChanged);
		Liberation->OnMilestone.AddDynamic(this, &UAzadiStyleSubsystem::HandleMilestone);
	}
}

void UAzadiStyleSubsystem::ApplyStylePack(FName StylePackId)
{
	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UAzadiModuleRegistry* Registry = GameInstance ? GameInstance->GetSubsystem<UAzadiModuleRegistry>() : nullptr;
	const FAzadiStylePackDef* Found = Registry ? Registry->FindStylePack(StylePackId) : nullptr;
	if (!Found)
	{
		UE_LOG(LogAzadi, Warning, TEXT("Style: pack '%s' not found, keeping defaults"), *StylePackId.ToString());
		Style = FAzadiStylePackDef();
	}
	else
	{
		Style = *Found;
	}
	bStyleApplied = true;

	HudFont = Style.FontPath.IsEmpty() ? nullptr : LoadObject<UFont>(nullptr, *Style.FontPath);

	EnsurePostProcess();
	const UAzadiLiberationSubsystem* Liberation = GetWorld()->GetSubsystem<UAzadiLiberationSubsystem>();
	ApplyGrade(Liberation ? Liberation->GetLiberation() : 0.f);
	StartMusic();

	UE_LOG(LogAzadi, Log, TEXT("Style: applied pack '%s'"), *Style.Id.ToString());
}

UFont* UAzadiStyleSubsystem::GetHudFont() const
{
	return HudFont ? HudFont.Get() : (GEngine ? GEngine->GetMediumFont() : nullptr);
}

void UAzadiStyleSubsystem::PlayStinger(FName StingerId)
{
	const FString* Ref = Style.Stingers.Find(StingerId);
	if (!Ref)
	{
		return;
	}
	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UAzadiModuleRegistry* Registry = GameInstance ? GameInstance->GetSubsystem<UAzadiModuleRegistry>() : nullptr;
	if (USoundBase* Sound = Registry ? Registry->ResolveSound(*Ref) : nullptr)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), Sound);
	}
}

void UAzadiStyleSubsystem::HandleLiberationChanged(float NewValue)
{
	if (bStyleApplied)
	{
		ApplyGrade(NewValue);
	}
}

void UAzadiStyleSubsystem::HandleCombatStateChanged(bool bInCombat)
{
	if (CombatMusic)
	{
		CombatMusic->AdjustVolume(bInCombat ? 0.5f : 1.5f, bInCombat ? 1.f : 0.f);
	}
	if (CalmMusic)
	{
		CalmMusic->AdjustVolume(bInCombat ? 0.5f : 1.5f, bInCombat ? 0.3f : 1.f);
	}
}

void UAzadiStyleSubsystem::HandleMilestone(float Milestone)
{
	if (Milestone >= 1.f)
	{
		// The dawn rises: fade the loops, play the anthem.
		if (CalmMusic) { CalmMusic->AdjustVolume(2.f, 0.f); }
		if (CombatMusic) { CombatMusic->AdjustVolume(2.f, 0.f); }

		const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
		UAzadiModuleRegistry* Registry = GameInstance ? GameInstance->GetSubsystem<UAzadiModuleRegistry>() : nullptr;
		if (USoundBase* Anthem = Registry ? Registry->ResolveSound(Style.MusicAnthem) : nullptr)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), Anthem);
		}
	}
	PlayStinger(TEXT("milestone"));
}

void UAzadiStyleSubsystem::EnsurePostProcess()
{
	if (PostProcess)
	{
		return;
	}
	AActor* Host = GetWorld()->SpawnActor<AActor>();
	if (!Host)
	{
		return;
	}
#if WITH_EDITOR
	Host->SetActorLabel(TEXT("AzadiStyleGrade"));
#endif
	PostProcess = NewObject<UPostProcessComponent>(Host, TEXT("AzadiGrade"));
	PostProcess->bUnbound = true;
	PostProcess->Priority = 10.f;
	PostProcess->RegisterComponent();
}

void UAzadiStyleSubsystem::ApplyGrade(float Liberation)
{
	if (!PostProcess)
	{
		return;
	}

	const float Sat = FMath::Lerp(Style.Saturation, Style.LiberatedSaturation, Liberation);
	const FLinearColor Tint = FMath::Lerp(Style.ColorTint, Style.LiberatedTint, Liberation);

	FPostProcessSettings& S = PostProcess->Settings;
	S.bOverride_ColorSaturation = true;
	S.ColorSaturation = FVector4(Sat, Sat, Sat, 1.f);
	S.bOverride_ColorContrast = true;
	S.ColorContrast = FVector4(Style.Contrast, Style.Contrast, Style.Contrast, 1.f);
	S.bOverride_ColorGain = true;
	S.ColorGain = FVector4(Tint.R, Tint.G, Tint.B, 1.f);
	S.bOverride_VignetteIntensity = true;
	S.VignetteIntensity = Style.VignetteIntensity;
	S.bOverride_BloomIntensity = true;
	S.BloomIntensity = Style.BloomIntensity;
	S.bOverride_FilmGrainIntensity = true;
	S.FilmGrainIntensity = Style.FilmGrainIntensity;
	S.bOverride_AutoExposureMinBrightness = true;
	S.bOverride_AutoExposureMaxBrightness = true;
	S.AutoExposureMinBrightness = 0.8f;
	S.AutoExposureMaxBrightness = 1.2f;
}

void UAzadiStyleSubsystem::StartMusic()
{
	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UAzadiModuleRegistry* Registry = GameInstance ? GameInstance->GetSubsystem<UAzadiModuleRegistry>() : nullptr;
	if (!Registry)
	{
		return;
	}

	if (!CalmMusic)
	{
		if (USoundBase* Calm = Registry->ResolveSound(Style.MusicCalm))
		{
			CalmMusic = UGameplayStatics::SpawnSound2D(GetWorld(), Calm, 1.f, 1.f, 0.f, nullptr, false, false);
		}
	}
	if (!CombatMusic)
	{
		if (USoundBase* Combat = Registry->ResolveSound(Style.MusicCombat))
		{
			CombatMusic = UGameplayStatics::SpawnSound2D(GetWorld(), Combat, 0.01f, 1.f, 0.f, nullptr, false, false);
		}
	}
}
