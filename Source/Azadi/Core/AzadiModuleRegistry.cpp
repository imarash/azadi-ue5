// AZADI: Rise of the Dawn

#include "Core/AzadiModuleRegistry.h"
#include "Azadi.h"
#include "Data/AzadiDataAssets.h"
#include "Dom/JsonObject.h"
#include "Engine/AssetManager.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Sound/SoundBase.h"

void UAzadiModuleRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 1. Core JSON mirrors.
	const FString DataDir = FPaths::ProjectContentDir() / TEXT("Azadi/Data");
	const int32 Loaded = LoadDataDirectory(DataDir);
	UE_LOG(LogAzadi, Log, TEXT("Registry: %d defs from core JSON (%s)"), Loaded, *DataDir);

	// 2. Editor-authored PrimaryDataAssets override JSON once the asset scan is ready.
	if (UAssetManager::IsInitialized())
	{
		UAssetManager::CallOrRegister_OnCompletedInitialScan(
			FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &UAzadiModuleRegistry::LoadPrimaryDataAssets));
	}

	// 3. Mod packs are layered on top by the AzadiModKit plugin subsystem,
	//    which declares an InitializeDependency on this registry.
}

const FAzadiDistrictDef* UAzadiModuleRegistry::FindDistrictByMap(const FString& ShortMapName) const
{
	for (const TPair<FName, FAzadiDistrictDef>& Pair : Districts)
	{
		if (Pair.Value.Map.Equals(ShortMapName, ESearchCase::IgnoreCase))
		{
			return &Pair.Value;
		}
	}
	return nullptr;
}

TArray<FName> UAzadiModuleRegistry::GetDistrictIds() const
{
	TArray<FName> Ids;
	Districts.GetKeys(Ids);
	return Ids;
}

void UAzadiModuleRegistry::RegisterCampaign(const FAzadiCampaignDef& Def)
{
	Campaign = Def;
	OnRegistryChanged.Broadcast();
}

void UAzadiModuleRegistry::RegisterDistrict(const FAzadiDistrictDef& Def)
{
	if (Def.Id.IsNone())
	{
		UE_LOG(LogAzadi, Warning, TEXT("Registry: district def without id ignored"));
		return;
	}
	Districts.Add(Def.Id, Def);
	OnRegistryChanged.Broadcast();
}

void UAzadiModuleRegistry::RegisterWeapon(const FAzadiWeaponDef& Def)
{
	if (!Def.Id.IsNone()) { Weapons.Add(Def.Id, Def); OnRegistryChanged.Broadcast(); }
}

void UAzadiModuleRegistry::RegisterEnemy(const FAzadiEnemyDef& Def)
{
	if (!Def.Id.IsNone()) { Enemies.Add(Def.Id, Def); OnRegistryChanged.Broadcast(); }
}

void UAzadiModuleRegistry::RegisterStylePack(const FAzadiStylePackDef& Def)
{
	if (!Def.Id.IsNone()) { StylePacks.Add(Def.Id, Def); OnRegistryChanged.Broadcast(); }
}

void UAzadiModuleRegistry::RegisterSignageSet(const FAzadiSignageSetDef& Def)
{
	if (!Def.Id.IsNone()) { SignageSets.Add(Def.Id, Def); OnRegistryChanged.Broadcast(); }
}

void UAzadiModuleRegistry::RegisterAssetPack(const FAzadiAssetPackDef& Def)
{
	if (!Def.Id.IsNone()) { AssetPacks.Add(Def.Id, Def); OnRegistryChanged.Broadcast(); }
}

void UAzadiModuleRegistry::InsertCampaignDistricts(const TArray<FName>& DistrictIds, FName InsertAfter)
{
	int32 InsertIndex = Campaign.Districts.Num();
	if (!InsertAfter.IsNone())
	{
		const int32 Found = Campaign.Districts.IndexOfByKey(InsertAfter);
		if (Found != INDEX_NONE)
		{
			InsertIndex = Found + 1;
		}
	}
	for (int32 i = 0; i < DistrictIds.Num(); ++i)
	{
		if (!Campaign.Districts.Contains(DistrictIds[i]))
		{
			Campaign.Districts.Insert(DistrictIds[i], InsertIndex + i);
		}
	}
	OnRegistryChanged.Broadcast();
}

int32 UAzadiModuleRegistry::LoadDataFile(const FString& JsonFilePath)
{
	FString Raw;
	if (!FFileHelper::LoadFileToString(Raw, *JsonFilePath))
	{
		UE_LOG(LogAzadi, Warning, TEXT("Registry: cannot read %s"), *JsonFilePath);
		return 0;
	}

	TSharedPtr<FJsonObject> Root;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		UE_LOG(LogAzadi, Warning, TEXT("Registry: invalid JSON in %s"), *JsonFilePath);
		return 0;
	}

	const FString Type = Root->GetStringField(TEXT("type")).ToLower();
	const TArray<TSharedPtr<FJsonValue>>* Items = nullptr;
	if (!Root->TryGetArrayField(TEXT("items"), Items) || Items == nullptr)
	{
		UE_LOG(LogAzadi, Warning, TEXT("Registry: %s has no items[]"), *JsonFilePath);
		return 0;
	}

	const FString BaseDir = FPaths::GetPath(JsonFilePath);
	int32 Count = 0;

	for (const TSharedPtr<FJsonValue>& Item : *Items)
	{
		const TSharedPtr<FJsonObject>* Obj = nullptr;
		if (!Item.IsValid() || !Item->TryGetObject(Obj) || Obj == nullptr)
		{
			continue;
		}
		const TSharedRef<FJsonObject> ObjRef = Obj->ToSharedRef();

		if (Type == TEXT("campaign"))
		{
			FAzadiCampaignDef Def;
			if (FJsonObjectConverter::JsonObjectToUStruct(ObjRef, &Def)) { RegisterCampaign(Def); ++Count; }
		}
		else if (Type == TEXT("district"))
		{
			FAzadiDistrictDef Def;
			if (FJsonObjectConverter::JsonObjectToUStruct(ObjRef, &Def)) { RegisterDistrict(Def); ++Count; }
		}
		else if (Type == TEXT("weapon"))
		{
			FAzadiWeaponDef Def;
			if (FJsonObjectConverter::JsonObjectToUStruct(ObjRef, &Def)) { RegisterWeapon(Def); ++Count; }
		}
		else if (Type == TEXT("enemy"))
		{
			FAzadiEnemyDef Def;
			if (FJsonObjectConverter::JsonObjectToUStruct(ObjRef, &Def)) { RegisterEnemy(Def); ++Count; }
		}
		else if (Type == TEXT("stylepack"))
		{
			FAzadiStylePackDef Def;
			if (FJsonObjectConverter::JsonObjectToUStruct(ObjRef, &Def)) { RegisterStylePack(Def); ++Count; }
		}
		else if (Type == TEXT("signageset"))
		{
			FAzadiSignageSetDef Def;
			if (FJsonObjectConverter::JsonObjectToUStruct(ObjRef, &Def))
			{
				NormalizeSignageRefs(Def, BaseDir);
				RegisterSignageSet(Def);
				++Count;
			}
		}
		else if (Type == TEXT("assetpack"))
		{
			FAzadiAssetPackDef Def;
			if (FJsonObjectConverter::JsonObjectToUStruct(ObjRef, &Def)) { RegisterAssetPack(Def); ++Count; }
		}
		else
		{
			UE_LOG(LogAzadi, Warning, TEXT("Registry: unknown data type '%s' in %s"), *Type, *JsonFilePath);
			break;
		}
	}
	return Count;
}

int32 UAzadiModuleRegistry::LoadDataDirectory(const FString& Directory)
{
	TArray<FString> Files;
	IFileManager::Get().FindFilesRecursive(Files, *Directory, TEXT("*.json"), true, false);
	Files.Sort();

	int32 Count = 0;
	for (const FString& File : Files)
	{
		Count += LoadDataFile(File);
	}
	return Count;
}

void UAzadiModuleRegistry::LoadPrimaryDataAssets()
{
	UAssetManager& Manager = UAssetManager::Get();
	int32 Count = 0;

	auto LoadType = [&Manager, &Count](FName TypeName, auto Visitor)
	{
		TArray<FPrimaryAssetId> Ids;
		Manager.GetPrimaryAssetIdList(FPrimaryAssetType(TypeName), Ids);
		for (const FPrimaryAssetId& Id : Ids)
		{
			const FSoftObjectPath Path = Manager.GetPrimaryAssetPath(Id);
			if (UObject* Asset = Path.TryLoad())
			{
				Visitor(Asset);
				++Count;
			}
		}
	};

	LoadType(TEXT("AzadiCampaign"), [this](UObject* A) { if (auto* D = Cast<UAzadiCampaignDataAsset>(A)) { RegisterCampaign(D->Def); } });
	LoadType(TEXT("AzadiDistrict"), [this](UObject* A) { if (auto* D = Cast<UAzadiDistrictDataAsset>(A)) { RegisterDistrict(D->Def); } });
	LoadType(TEXT("AzadiWeapon"), [this](UObject* A) { if (auto* D = Cast<UAzadiWeaponDataAsset>(A)) { RegisterWeapon(D->Def); } });
	LoadType(TEXT("AzadiEnemy"), [this](UObject* A) { if (auto* D = Cast<UAzadiEnemyDataAsset>(A)) { RegisterEnemy(D->Def); } });
	LoadType(TEXT("AzadiStylePack"), [this](UObject* A) { if (auto* D = Cast<UAzadiStylePackDataAsset>(A)) { RegisterStylePack(D->Def); } });
	LoadType(TEXT("AzadiSignageSet"), [this](UObject* A) { if (auto* D = Cast<UAzadiSignageSetDataAsset>(A)) { RegisterSignageSet(D->Def); } });
	LoadType(TEXT("AzadiAssetPack"), [this](UObject* A) { if (auto* D = Cast<UAzadiAssetPackDataAsset>(A)) { RegisterAssetPack(D->Def); } });

	if (Count > 0)
	{
		UE_LOG(LogAzadi, Log, TEXT("Registry: %d defs from PrimaryDataAssets"), Count);
	}
}

void UAzadiModuleRegistry::NormalizeSignageRefs(FAzadiSignageSetDef& Def, const FString& BaseDir)
{
	auto Normalize = [&BaseDir](FString& Ref)
	{
		if (Ref.StartsWith(TEXT("file:")))
		{
			const FString Relative = Ref.RightChop(5);
			if (FPaths::IsRelative(Relative))
			{
				Ref = TEXT("file:") + FPaths::ConvertRelativePathToFull(BaseDir / Relative);
			}
		}
	};
	for (FAzadiSignageSlotDef& Slot : Def.Slots)
	{
		Normalize(Slot.OccupiedTexture);
		Normalize(Slot.LiberatedTexture);
	}
}

UTexture2D* UAzadiModuleRegistry::ResolveTexture(const FString& Ref)
{
	if (Ref.IsEmpty())
	{
		return nullptr;
	}
	if (Ref.StartsWith(TEXT("file:")))
	{
		const FString FilePath = Ref.RightChop(5);
		if (TObjectPtr<UTexture2D>* Cached = ImportedTextures.Find(FilePath))
		{
			return *Cached;
		}
		UTexture2D* Imported = FImageUtils::ImportFileAsTexture2D(FilePath);
		if (Imported)
		{
			ImportedTextures.Add(FilePath, Imported);
		}
		else
		{
			UE_LOG(LogAzadi, Warning, TEXT("Registry: failed to import texture %s"), *FilePath);
		}
		return Imported;
	}
	return LoadObject<UTexture2D>(nullptr, *Ref);
}

USoundBase* UAzadiModuleRegistry::ResolveSound(const FString& Ref) const
{
	return Ref.IsEmpty() ? nullptr : LoadObject<USoundBase>(nullptr, *Ref);
}

UStaticMesh* UAzadiModuleRegistry::ResolveMesh(const FString& Ref) const
{
	return Ref.IsEmpty() ? nullptr : LoadObject<UStaticMesh>(nullptr, *Ref);
}

FLinearColor UAzadiModuleRegistry::ParseColorHex(const FString& Hex, const FLinearColor& Fallback)
{
	if (Hex.IsEmpty())
	{
		return Fallback;
	}
	const FColor Color = FColor::FromHex(Hex);
	return (Color == FColor(ForceInitToZero) && Hex != TEXT("#000000") && Hex != TEXT("000000"))
		? Fallback
		: FLinearColor::FromSRGBColor(Color);
}
