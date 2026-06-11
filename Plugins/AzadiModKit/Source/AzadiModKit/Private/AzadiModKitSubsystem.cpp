// AZADI: Rise of the Dawn — ModKit

#include "AzadiModKitSubsystem.h"
#include "Core/AzadiModuleRegistry.h"
#include "HAL/FileManager.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY_STATIC(LogAzadiModKit, Log, All);

void UAzadiModKitSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Core content first, mods on top.
	Registry = Collection.InitializeDependency<UAzadiModuleRegistry>();
	if (Registry)
	{
		DiscoverAndLoad();
	}
}

void UAzadiModKitSubsystem::DiscoverAndLoad()
{
	const FString Roots[] = {
		FPaths::ProjectDir() / TEXT("Mods"),
		FPaths::ProjectContentDir() / TEXT("Mods")
	};

	TArray<FDiscoveredMod> Discovered;
	for (const FString& Root : Roots)
	{
		TArray<FString> ModDirs;
		IFileManager::Get().FindFiles(ModDirs, *(Root / TEXT("*")), false, true);
		ModDirs.Sort();

		for (const FString& DirName : ModDirs)
		{
			const FString ModDir = Root / DirName;
			const FString ManifestPath = ModDir / TEXT("manifest.json");

			FString Raw;
			if (!FFileHelper::LoadFileToString(Raw, *ManifestPath))
			{
				continue;
			}

			FDiscoveredMod Mod;
			Mod.Directory = ModDir;
			if (!FJsonObjectConverter::JsonObjectStringToUStruct(Raw, &Mod.Manifest))
			{
				UE_LOG(LogAzadiModKit, Warning, TEXT("ModKit: invalid manifest %s"), *ManifestPath);
				continue;
			}
			if (Mod.Manifest.Id.IsEmpty())
			{
				UE_LOG(LogAzadiModKit, Warning, TEXT("ModKit: manifest without id at %s"), *ManifestPath);
				continue;
			}
			if (!Mod.Manifest.Enabled)
			{
				UE_LOG(LogAzadiModKit, Log, TEXT("ModKit: '%s' present but disabled (set \"enabled\": true to activate)"), *Mod.Manifest.Id);
				continue;
			}
			Discovered.Add(MoveTemp(Mod));
		}
	}

	if (Discovered.IsEmpty())
	{
		return;
	}

	// Dependency-ordered load: keep sweeping until no progress.
	TSet<FString> LoadedIds;
	bool bProgress = true;
	while (bProgress)
	{
		bProgress = false;
		for (int32 i = Discovered.Num() - 1; i >= 0; --i)
		{
			const FDiscoveredMod& Mod = Discovered[i];
			const bool bDepsMet = !Mod.Manifest.Dependencies.ContainsByPredicate(
				[&LoadedIds](const FString& Dep) { return !LoadedIds.Contains(Dep); });
			if (!bDepsMet)
			{
				continue;
			}
			if (LoadMod(Mod))
			{
				LoadedIds.Add(Mod.Manifest.Id);
				LoadedMods.Add(Mod.Manifest);
			}
			Discovered.RemoveAt(i);
			bProgress = true;
		}
	}

	for (const FDiscoveredMod& Skipped : Discovered)
	{
		UE_LOG(LogAzadiModKit, Warning, TEXT("ModKit: '%s' skipped — missing dependencies (%s)"),
			*Skipped.Manifest.Id, *FString::Join(Skipped.Manifest.Dependencies, TEXT(", ")));
	}

	UE_LOG(LogAzadiModKit, Log, TEXT("ModKit: %d mod pack(s) active"), LoadedMods.Num());
}

bool UAzadiModKitSubsystem::LoadMod(const FDiscoveredMod& Mod)
{
	UE_LOG(LogAzadiModKit, Log, TEXT("ModKit: loading '%s' v%s by %s"),
		*Mod.Manifest.Id, *Mod.Manifest.Version, *Mod.Manifest.Author);

	int32 Items = 0;
	if (Mod.Manifest.Data.Num() > 0)
	{
		for (const FString& DataFile : Mod.Manifest.Data)
		{
			Items += Registry->LoadDataFile(Mod.Directory / DataFile);
		}
	}
	else
	{
		// No explicit list: load every *.json under the mod's data/ folder.
		Items += Registry->LoadDataDirectory(Mod.Directory / TEXT("data"));
	}

	if (Mod.Manifest.CampaignAppend.Num() > 0)
	{
		TArray<FName> DistrictIds;
		for (const FString& Id : Mod.Manifest.CampaignAppend)
		{
			DistrictIds.Add(FName(*Id));
		}
		Registry->InsertCampaignDistricts(DistrictIds, FName(*Mod.Manifest.CampaignInsertAfter));
	}

	UE_LOG(LogAzadiModKit, Log, TEXT("ModKit: '%s' registered %d defs"), *Mod.Manifest.Id, Items);
	return true;
}
