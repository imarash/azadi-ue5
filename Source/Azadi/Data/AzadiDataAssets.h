// AZADI: Rise of the Dawn
//
// Thin UPrimaryDataAsset wrappers around the F*Def schema (AzadiTypes.h).
// Editor-authored assets and JSON mirrors are interchangeable: the
// UAzadiModuleRegistry merges both by id (assets override JSON).

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/AzadiTypes.h"
#include "AzadiDataAssets.generated.h"

UCLASS(BlueprintType)
class AZADI_API UAzadiCampaignDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FAzadiCampaignDef Def;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("AzadiCampaign"), Def.Id.IsNone() ? GetFName() : Def.Id);
	}
};

UCLASS(BlueprintType)
class AZADI_API UAzadiDistrictDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FAzadiDistrictDef Def;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("AzadiDistrict"), Def.Id.IsNone() ? GetFName() : Def.Id);
	}
};

UCLASS(BlueprintType)
class AZADI_API UAzadiWeaponDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FAzadiWeaponDef Def;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("AzadiWeapon"), Def.Id.IsNone() ? GetFName() : Def.Id);
	}
};

UCLASS(BlueprintType)
class AZADI_API UAzadiEnemyDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FAzadiEnemyDef Def;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("AzadiEnemy"), Def.Id.IsNone() ? GetFName() : Def.Id);
	}
};

UCLASS(BlueprintType)
class AZADI_API UAzadiStylePackDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FAzadiStylePackDef Def;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("AzadiStylePack"), Def.Id.IsNone() ? GetFName() : Def.Id);
	}
};

UCLASS(BlueprintType)
class AZADI_API UAzadiSignageSetDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FAzadiSignageSetDef Def;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("AzadiSignageSet"), Def.Id.IsNone() ? GetFName() : Def.Id);
	}
};

UCLASS(BlueprintType)
class AZADI_API UAzadiAssetPackDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FAzadiAssetPackDef Def;

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("AzadiAssetPack"), Def.Id.IsNone() ? GetFName() : Def.Id);
	}
};
