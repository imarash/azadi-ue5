// AZADI: Rise of the Dawn — ModKit

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AzadiModKitSubsystem.generated.h"

class UAzadiModuleRegistry;

/** manifest.json schema for a mod pack. See docs/MODDING.md. */
USTRUCT()
struct FAzadiModManifest
{
	GENERATED_BODY()

	UPROPERTY() FString Id;
	UPROPERTY() FString Name;
	UPROPERTY() FString Version;
	UPROPERTY() FString Author;
	UPROPERTY() FString Description;

	/** Set false to ship a pack dormant; players flip it to opt in. */
	UPROPERTY() bool Enabled = true;

	/** Ids of mods that must load before this one. */
	UPROPERTY() TArray<FString> Dependencies;

	/** Data files (relative to the mod folder) in load order. */
	UPROPERTY() TArray<FString> Data;

	/** District ids to splice into the campaign order. */
	UPROPERTY() TArray<FString> CampaignAppend;

	/** Optional anchor: insert CampaignAppend after this district id. */
	UPROPERTY() FString CampaignInsertAfter;
};

/**
 * Discovers mod packs and layers them onto the UAzadiModuleRegistry after
 * core content has loaded. A pack is any folder with a manifest.json under:
 *
 *   <Project>/Mods/<modId>/            (loose packs, dev + shipped)
 *   <Project>/Content/Mods/<modId>/    (packs that also carry .uasset content)
 *
 * Dependency-ordered; unresolved dependencies are skipped with a warning.
 */
UCLASS()
class AZADIMODKIT_API UAzadiModKitSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	const TArray<FAzadiModManifest>& GetLoadedMods() const { return LoadedMods; }

private:
	struct FDiscoveredMod
	{
		FAzadiModManifest Manifest;
		FString Directory;
	};

	void DiscoverAndLoad();
	bool LoadMod(const FDiscoveredMod& Mod);

	UPROPERTY(Transient)
	TObjectPtr<UAzadiModuleRegistry> Registry;

	TArray<FAzadiModManifest> LoadedMods;
};
