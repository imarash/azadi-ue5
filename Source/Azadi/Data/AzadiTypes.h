// AZADI: Rise of the Dawn
//
// The single schema for all tunable game data. These plain structs are:
//   1. deserialized from JSON mirrors (Content/Azadi/Data/**.json),
//   2. wrapped by UPrimaryDataAsset types for editor authoring,
//   3. registered by mod packs through the AzadiModKit plugin.
//
// Designers and modders tune these — never C++ constants.

#pragma once

#include "CoreMinimal.h"
#include "AzadiTypes.generated.h"

/** How a weapon delivers its effect. */
UENUM(BlueprintType)
enum class EAzadiWeaponKind : uint8
{
	Hitscan,
	Projectile,
	Melee,
	Chant        // megaphone: radial shockwave
};

/** How an enemy attacks. */
UENUM(BlueprintType)
enum class EAzadiAttackKind : uint8
{
	Melee,
	Ranged
};

/** District completion rule. */
UENUM(BlueprintType)
enum class EAzadiObjective : uint8
{
	ReachExit,      // touch the exit zone
	Liberate,       // AZADI meter reaches 1.0
	Rescue,         // free all citizen cages
	Clear           // kill all spawned enemies
};

/** One entry of a district spawn table. */
USTRUCT(BlueprintType)
struct FAzadiSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FName EnemyId;

	/** Relative likelihood when the spawner rolls this table. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float Weight = 1.f;
};

/** Weapon tuning. All times in seconds, distances in cm, angles in degrees. */
USTRUCT(BlueprintType)
struct FAzadiWeaponDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	EAzadiWeaponKind Kind = EAzadiWeaponKind::Hitscan;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float FireInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	bool bAutomatic = false;

	/** Traces per shot (shotgun pellets). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	int32 Pellets = 1;

	/** Hip-fire cone half angle. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float SpreadDeg = 2.f;

	/** Spread multiplier while aiming down sights. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float AdsSpreadMul = 0.35f;

	/** Camera FOV while aiming down sights. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float AdsFov = 65.f;

	/** Rounds per magazine. 0 = no ammo management (melee/chant). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	int32 MagSize = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	int32 ReserveAmmo = 48;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float ReloadTime = 1.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float Range = 9000.f;

	/** Pitch kick per shot. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float RecoilPitch = 0.6f;

	/** Projectile weapons only. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float ProjectileSpeed = 2200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float SplashRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float SplashDamage = 60.f;

	/** Chant weapons: knockback impulse. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float ChantImpulse = 1200.f;

	/** Optional asset refs ("/Game/..." object paths). Empty = silent. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FString FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FString ReloadSound;
};

/** Enemy tuning. */
USTRUCT(BlueprintType)
struct FAzadiEnemyDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float Health = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float MoveSpeed = 380.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	EAzadiAttackKind AttackKind = EAzadiAttackKind::Melee;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float Damage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float AttackInterval = 1.2f;

	/** Range at which the enemy stops chasing and attacks. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float AttackRange = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float SightRadius = 3500.f;

	/** AZADI meter gain on death (0..1 scale). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float LiberationReward = 0.06f;

	/** Ranged enemies: projectile speed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float ProjectileSpeed = 1400.f;

	/** Uniform scale of the placeholder body (boss = big). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float Scale = 1.f;

	/** Placeholder body tint, "#RRGGBB". Replaced by MeshPath when skinned. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FString ColorHex = TEXT("#3A3A46");

	/** Optional skeletal/static mesh override ("/Game/..." object path). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FString MeshPath;
};

/** One signage surface: occupied propaganda vs. liberated mural. */
USTRUCT(BlueprintType)
struct FAzadiSignageSlotDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FName SlotId;

	/**
	 * Texture refs. Two forms:
	 *   "/Game/Path/To.Texture"  — content asset
	 *   "file:relative/path.png" — raw PNG imported at runtime (mod-friendly),
	 *                              relative to the defining JSON file
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FString OccupiedTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FString LiberatedTexture;
};

/** A named set of signage slots a district can reference. */
USTRUCT(BlueprintType)
struct FAzadiSignageSetDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	TArray<FAzadiSignageSlotDef> Slots;
};

/** HUD + color grade + audio stems. Swappable without code. */
USTRUCT(BlueprintType)
struct FAzadiStylePackDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FString DisplayName;

	// --- HUD skin ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FLinearColor HudAccent = FLinearColor(1.f, 0.82f, 0.25f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FLinearColor HudText = FLinearColor(0.95f, 0.95f, 0.92f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FLinearColor HudBackdrop = FLinearColor(0.02f, 0.02f, 0.03f, 0.55f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FLinearColor CrosshairColor = FLinearColor(0.95f, 0.95f, 0.92f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FLinearColor DangerColor = FLinearColor(0.85f, 0.16f, 0.12f);

	/** Optional UFont asset ref for HUD text. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FString FontPath;

	// --- Color grade (applied to an unbound post-process) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float Saturation = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float Contrast = 1.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FLinearColor ColorTint = FLinearColor(1.f, 0.97f, 0.92f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float VignetteIntensity = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float BloomIntensity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float FilmGrainIntensity = 0.12f;

	/** Saturation when the district is fully liberated (dawn rises). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float LiberatedSaturation = 1.15f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FLinearColor LiberatedTint = FLinearColor(1.05f, 1.f, 0.95f);

	// --- Audio stems (USoundBase asset refs) ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FString MusicCalm;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FString MusicCombat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FString MusicAnthem;

	/** Named one-shots: "cheer", "sign_swap", ... */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	TMap<FName, FString> Stingers;
};

/** Environment kit slot bindings: logical slot -> mesh/material asset ref. */
USTRUCT(BlueprintType)
struct FAzadiAssetPackDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FString DisplayName;

	/** e.g. "wall_bazaar" -> "/Game/Leartes/Meshes/SM_Wall_03.SM_Wall_03" */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	TMap<FName, FString> MeshSlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	TMap<FName, FString> MaterialSlots;
};

/** Per-district config: the unit of campaign modularity. */
USTRUCT(BlueprintType)
struct FAzadiDistrictDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FString DisplayName;

	/** Short map name, e.g. "L_Bazaar". Resolved under MapRoot. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FString Map;

	/** Long package root for this district's map. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FString MapRoot = TEXT("/Game/Azadi/Maps");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	EAzadiObjective Objective = EAzadiObjective::ReachExit;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FName StylePackId = TEXT("urban_grit");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FName AssetPackId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FName SignageSetId;

	/** Max enemies alive simultaneously across all spawners. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	int32 EnemyBudget = 8;

	/** Total enemies the district will spawn (waves refill up to budget). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	int32 TotalEnemies = 16;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	TArray<FAzadiSpawnEntry> SpawnTable;

	/** Weapon ids granted on district start. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	TArray<FName> Loadout;

	/** AZADI gained per citizen cage freed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float RescueReward = 0.15f;

	/** "dusk", "night", "noon" — applied to the tagged sun if present. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FName LightingPreset = TEXT("dusk");

	/** Meter fraction at which signage flips to liberated murals. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	float SignageFlipThreshold = 0.5f;
};

/** Campaign: ordered district ids. */
USTRUCT(BlueprintType)
struct FAzadiCampaignDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FName Id = TEXT("main");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Azadi")
	TArray<FName> Districts;
};
