// AZADI: Rise of the Dawn

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/AzadiTypes.h"
#include "AzadiWeaponComponent.generated.h"

class USoundBase;

/** Runtime state for one carried weapon. */
USTRUCT()
struct FAzadiWeaponSlot
{
	GENERATED_BODY()

	UPROPERTY()
	FAzadiWeaponDef Def;

	UPROPERTY()
	int32 Mag = 0;

	UPROPERTY()
	int32 Reserve = 0;

	UPROPERTY()
	TObjectPtr<USoundBase> FireSound = nullptr;

	UPROPERTY()
	TObjectPtr<USoundBase> ReloadSound = nullptr;
};

/**
 * Combat module: data-driven weapon handling for the player pawn.
 * Hitscan / projectile / melee / chant kinds, ADS, spread, ammo, reload —
 * all tuned from FAzadiWeaponDef (JSON / DataAsset / mod).
 */
UCLASS(ClassGroup = (Azadi), meta = (BlueprintSpawnableComponent))
class AZADI_API UAzadiWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAzadiWeaponComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Resolve weapon ids through the registry and equip the first. */
	void InitLoadout(const TArray<FName>& WeaponIds);

	void StartFire();
	void StopFire();
	void StartReload();
	void SetAds(bool bEnable);
	void CycleWeapon(int32 Direction);
	void EquipIndex(int32 Index);

	bool HasWeapons() const { return Slots.Num() > 0; }
	const FAzadiWeaponSlot* GetCurrentSlot() const;
	FString GetAmmoText() const;
	FString GetWeaponName() const;

	/** Effective cone half-angle right now (HUD crosshair + traces). */
	float GetCurrentSpreadDeg() const;

	bool IsAds() const { return bAds; }
	float GetAdsAlpha() const { return AdsAlpha; }

	/** Target camera FOV given ADS state (HipFov when not aiming). */
	float GetTargetFov(float HipFov) const;

	/** World seconds of the last confirmed hit on a hostile — HUD hitmarker. */
	float GetLastHitTime() const { return LastHitTime; }

	bool IsReloading() const { return bReloading; }

private:
	void TryFire();
	void FireOnce();
	void DoHitscan(const FAzadiWeaponDef& Def);
	void DoMelee(const FAzadiWeaponDef& Def);
	void DoChant(const FAzadiWeaponDef& Def);
	void DoProjectile(const FAzadiWeaponDef& Def);
	void FinishReload();
	void GetAimView(FVector& OutLocation, FRotator& OutRotation) const;
	void ApplyRecoil(const FAzadiWeaponDef& Def) const;
	void RegisterHit();

	UPROPERTY(Transient)
	TArray<FAzadiWeaponSlot> Slots;

	int32 CurrentIndex = 0;
	bool bWantsFire = false;
	bool bAds = false;
	bool bReloading = false;
	float AdsAlpha = 0.f;
	float NextShotTime = 0.f;
	float LastHitTime = -1000.f;

	FTimerHandle AutoFireTimer;
	FTimerHandle ReloadTimer;
};
