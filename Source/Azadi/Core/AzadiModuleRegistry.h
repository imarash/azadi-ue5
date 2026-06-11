// AZADI: Rise of the Dawn

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/AzadiTypes.h"
#include "AzadiModuleRegistry.generated.h"

class UTexture2D;
class USoundBase;
class UStaticMesh;

/**
 * Central content registry — the "UAzadiModule" hub every other system
 * resolves through. Content arrives from three sources, merged by id in
 * load order (later wins):
 *
 *   1. JSON mirrors      Content/Azadi/Data/ *.json     (text, diffable)
 *   2. PrimaryDataAssets /Game/Azadi/Data               (editor-authored)
 *   3. Mod packs         <Project>/Mods/<id>/           (AzadiModKit plugin)
 *
 * Data files are self-describing:
 *   { "type": "district", "items": [ { ... }, { ... } ] }
 * with type one of: campaign, district, weapon, enemy, stylepack,
 * signageset, assetpack.
 */
UCLASS()
class AZADI_API UAzadiModuleRegistry : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// --- Lookup ---
	const FAzadiCampaignDef& GetCampaign() const { return Campaign; }
	const FAzadiDistrictDef* FindDistrict(FName Id) const { return Districts.Find(Id); }
	const FAzadiDistrictDef* FindDistrictByMap(const FString& ShortMapName) const;
	const FAzadiWeaponDef* FindWeapon(FName Id) const { return Weapons.Find(Id); }
	const FAzadiEnemyDef* FindEnemy(FName Id) const { return Enemies.Find(Id); }
	const FAzadiStylePackDef* FindStylePack(FName Id) const { return StylePacks.Find(Id); }
	const FAzadiSignageSetDef* FindSignageSet(FName Id) const { return SignageSets.Find(Id); }
	const FAzadiAssetPackDef* FindAssetPack(FName Id) const { return AssetPacks.Find(Id); }
	TArray<FName> GetDistrictIds() const;

	// --- Registration (used by JSON loader, DataAssets, and mods) ---
	void RegisterCampaign(const FAzadiCampaignDef& Def);
	void RegisterDistrict(const FAzadiDistrictDef& Def);
	void RegisterWeapon(const FAzadiWeaponDef& Def);
	void RegisterEnemy(const FAzadiEnemyDef& Def);
	void RegisterStylePack(const FAzadiStylePackDef& Def);
	void RegisterSignageSet(const FAzadiSignageSetDef& Def);
	void RegisterAssetPack(const FAzadiAssetPackDef& Def);

	/** Mods: append districts to the campaign order (after a given id, or at the end). */
	void InsertCampaignDistricts(const TArray<FName>& DistrictIds, FName InsertAfter = NAME_None);

	// --- Data loading ---
	/** Load one self-describing JSON data file. Returns number of items registered. */
	int32 LoadDataFile(const FString& JsonFilePath);

	/** Recursively load all *.json under a directory. Returns number of items registered. */
	int32 LoadDataDirectory(const FString& Directory);

	// --- Asset reference resolution ---
	/** "/Game/...": load content asset. "file:<path>.png": import raw PNG (cached). */
	UTexture2D* ResolveTexture(const FString& Ref);
	USoundBase* ResolveSound(const FString& Ref) const;
	UStaticMesh* ResolveMesh(const FString& Ref) const;

	static FLinearColor ParseColorHex(const FString& Hex, const FLinearColor& Fallback = FLinearColor::White);

	/** Fired whenever new content is registered (mods loading late, etc.). */
	FSimpleMulticastDelegate OnRegistryChanged;

private:
	void LoadPrimaryDataAssets();

	/** Rewrite "file:" refs inside a signage set to absolute paths. */
	static void NormalizeSignageRefs(FAzadiSignageSetDef& Def, const FString& BaseDir);

	FAzadiCampaignDef Campaign;
	TMap<FName, FAzadiDistrictDef> Districts;
	TMap<FName, FAzadiWeaponDef> Weapons;
	TMap<FName, FAzadiEnemyDef> Enemies;
	TMap<FName, FAzadiStylePackDef> StylePacks;
	TMap<FName, FAzadiSignageSetDef> SignageSets;
	TMap<FName, FAzadiAssetPackDef> AssetPacks;

	/** Keeps runtime-imported PNG textures alive. */
	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<UTexture2D>> ImportedTextures;
};
