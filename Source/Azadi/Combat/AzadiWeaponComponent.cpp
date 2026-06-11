// AZADI: Rise of the Dawn

#include "Combat/AzadiWeaponComponent.h"
#include "Azadi.h"
#include "Combat/AzadiHealthComponent.h"
#include "Combat/AzadiProjectile.h"
#include "Core/AzadiModuleRegistry.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

UAzadiWeaponComponent::UAzadiWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAzadiWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	AdsAlpha = FMath::FInterpTo(AdsAlpha, bAds ? 1.f : 0.f, DeltaTime, 12.f);
}

void UAzadiWeaponComponent::InitLoadout(const TArray<FName>& WeaponIds)
{
	const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	UAzadiModuleRegistry* Registry = GameInstance ? GameInstance->GetSubsystem<UAzadiModuleRegistry>() : nullptr;
	if (!Registry)
	{
		return;
	}

	Slots.Reset();
	for (const FName& Id : WeaponIds)
	{
		const FAzadiWeaponDef* Def = Registry->FindWeapon(Id);
		if (!Def)
		{
			UE_LOG(LogAzadi, Warning, TEXT("Weapons: unknown weapon id '%s'"), *Id.ToString());
			continue;
		}
		FAzadiWeaponSlot& Slot = Slots.AddDefaulted_GetRef();
		Slot.Def = *Def;
		Slot.Mag = Def->MagSize;
		Slot.Reserve = Def->ReserveAmmo;
		Slot.FireSound = Registry->ResolveSound(Def->FireSound);
		Slot.ReloadSound = Registry->ResolveSound(Def->ReloadSound);
	}
	CurrentIndex = 0;
	UE_LOG(LogAzadi, Log, TEXT("Weapons: loadout of %d equipped"), Slots.Num());
}

const FAzadiWeaponSlot* UAzadiWeaponComponent::GetCurrentSlot() const
{
	return Slots.IsValidIndex(CurrentIndex) ? &Slots[CurrentIndex] : nullptr;
}

FString UAzadiWeaponComponent::GetAmmoText() const
{
	const FAzadiWeaponSlot* Slot = GetCurrentSlot();
	if (!Slot)
	{
		return FString();
	}
	if (Slot->Def.MagSize <= 0)
	{
		return TEXT("--");
	}
	return FString::Printf(TEXT("%d / %d"), Slot->Mag, Slot->Reserve);
}

FString UAzadiWeaponComponent::GetWeaponName() const
{
	const FAzadiWeaponSlot* Slot = GetCurrentSlot();
	return Slot ? Slot->Def.DisplayName : FString();
}

float UAzadiWeaponComponent::GetCurrentSpreadDeg() const
{
	const FAzadiWeaponSlot* Slot = GetCurrentSlot();
	if (!Slot)
	{
		return 0.f;
	}
	float Spread = Slot->Def.SpreadDeg;
	Spread *= FMath::Lerp(1.f, Slot->Def.AdsSpreadMul, AdsAlpha);

	if (const AActor* Owner = GetOwner())
	{
		if (Owner->GetVelocity().Size2D() > 60.f)
		{
			Spread *= 1.4f;
		}
	}
	return Spread;
}

float UAzadiWeaponComponent::GetTargetFov(float HipFov) const
{
	const FAzadiWeaponSlot* Slot = GetCurrentSlot();
	const float AdsFov = Slot ? Slot->Def.AdsFov : HipFov;
	return FMath::Lerp(HipFov, AdsFov, AdsAlpha);
}

void UAzadiWeaponComponent::StartFire()
{
	bWantsFire = true;
	TryFire();

	const FAzadiWeaponSlot* Slot = GetCurrentSlot();
	if (Slot && Slot->Def.bAutomatic)
	{
		GetWorld()->GetTimerManager().SetTimer(AutoFireTimer, this, &UAzadiWeaponComponent::TryFire,
			FMath::Max(0.02f, Slot->Def.FireInterval), true);
	}
}

void UAzadiWeaponComponent::StopFire()
{
	bWantsFire = false;
	GetWorld()->GetTimerManager().ClearTimer(AutoFireTimer);
}

void UAzadiWeaponComponent::SetAds(bool bEnable)
{
	bAds = bEnable;
}

void UAzadiWeaponComponent::CycleWeapon(int32 Direction)
{
	if (Slots.Num() < 2)
	{
		return;
	}
	EquipIndex((CurrentIndex + Direction + Slots.Num()) % Slots.Num());
}

void UAzadiWeaponComponent::EquipIndex(int32 Index)
{
	if (!Slots.IsValidIndex(Index) || Index == CurrentIndex)
	{
		return;
	}
	StopFire();
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
	bReloading = false;
	CurrentIndex = Index;
}

void UAzadiWeaponComponent::StartReload()
{
	FAzadiWeaponSlot* Slot = Slots.IsValidIndex(CurrentIndex) ? &Slots[CurrentIndex] : nullptr;
	if (!Slot || bReloading || Slot->Def.MagSize <= 0 || Slot->Mag == Slot->Def.MagSize || Slot->Reserve <= 0)
	{
		return;
	}
	bReloading = true;
	if (Slot->ReloadSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), Slot->ReloadSound);
	}
	GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &UAzadiWeaponComponent::FinishReload,
		FMath::Max(0.1f, Slot->Def.ReloadTime), false);
}

void UAzadiWeaponComponent::FinishReload()
{
	bReloading = false;
	FAzadiWeaponSlot* Slot = Slots.IsValidIndex(CurrentIndex) ? &Slots[CurrentIndex] : nullptr;
	if (!Slot)
	{
		return;
	}
	const int32 Needed = Slot->Def.MagSize - Slot->Mag;
	const int32 Taken = FMath::Min(Needed, Slot->Reserve);
	Slot->Mag += Taken;
	Slot->Reserve -= Taken;
}

void UAzadiWeaponComponent::TryFire()
{
	if (!bWantsFire || bReloading)
	{
		return;
	}
	FAzadiWeaponSlot* Slot = Slots.IsValidIndex(CurrentIndex) ? &Slots[CurrentIndex] : nullptr;
	if (!Slot)
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now < NextShotTime)
	{
		return;
	}

	if (Slot->Def.MagSize > 0 && Slot->Mag <= 0)
	{
		StartReload();
		return;
	}

	NextShotTime = Now + Slot->Def.FireInterval;
	if (Slot->Def.MagSize > 0)
	{
		--Slot->Mag;
	}
	FireOnce();

	if (!Slot->Def.bAutomatic)
	{
		bWantsFire = false;
	}
}

void UAzadiWeaponComponent::FireOnce()
{
	const FAzadiWeaponSlot* Slot = GetCurrentSlot();
	if (!Slot)
	{
		return;
	}
	const FAzadiWeaponDef& Def = Slot->Def;

	switch (Def.Kind)
	{
	case EAzadiWeaponKind::Hitscan:    DoHitscan(Def); break;
	case EAzadiWeaponKind::Projectile: DoProjectile(Def); break;
	case EAzadiWeaponKind::Melee:      DoMelee(Def); break;
	case EAzadiWeaponKind::Chant:      DoChant(Def); break;
	}

	if (Slot->FireSound)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), Slot->FireSound);
	}
	ApplyRecoil(Def);
}

void UAzadiWeaponComponent::DoHitscan(const FAzadiWeaponDef& Def)
{
	FVector ViewLocation;
	FRotator ViewRotation;
	GetAimView(ViewLocation, ViewRotation);

	const float SpreadRad = FMath::DegreesToRadians(GetCurrentSpreadDeg());
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AzadiHitscan), false, GetOwner());

	for (int32 Pellet = 0; Pellet < FMath::Max(1, Def.Pellets); ++Pellet)
	{
		const FVector Dir = FMath::VRandCone(ViewRotation.Vector(), SpreadRad);
		const FVector End = ViewLocation + Dir * Def.Range;

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, ViewLocation, End, ECC_Pawn, Params))
		{
			if (AActor* HitActor = Hit.GetActor())
			{
				APawn* OwnerPawn = Cast<APawn>(GetOwner());
				UGameplayStatics::ApplyPointDamage(HitActor, Def.Damage, Dir, Hit,
					OwnerPawn ? OwnerPawn->GetController() : nullptr, GetOwner(), nullptr);

				if (HitActor->FindComponentByClass<UAzadiHealthComponent>())
				{
					RegisterHit();
				}
			}
		}
	}
}

void UAzadiWeaponComponent::DoMelee(const FAzadiWeaponDef& Def)
{
	FVector ViewLocation;
	FRotator ViewRotation;
	GetAimView(ViewLocation, ViewRotation);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AzadiMelee), false, GetOwner());
	FHitResult Hit;
	const FVector End = ViewLocation + ViewRotation.Vector() * Def.Range;
	if (GetWorld()->LineTraceSingleByChannel(Hit, ViewLocation, End, ECC_Pawn, Params))
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			APawn* OwnerPawn = Cast<APawn>(GetOwner());
			UGameplayStatics::ApplyPointDamage(HitActor, Def.Damage, ViewRotation.Vector(), Hit,
				OwnerPawn ? OwnerPawn->GetController() : nullptr, GetOwner(), nullptr);

			if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
			{
				HitCharacter->LaunchCharacter(ViewRotation.Vector() * Def.ChantImpulse * 0.3f, false, false);
			}
			if (HitActor->FindComponentByClass<UAzadiHealthComponent>())
			{
				RegisterHit();
			}
		}
	}
}

void UAzadiWeaponComponent::DoChant(const FAzadiWeaponDef& Def)
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}
	const FVector Center = Owner->GetActorLocation();
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	AController* OwnerController = OwnerPawn ? OwnerPawn->GetController() : nullptr;

	bool bAnyHit = false;
	for (TObjectIterator<UAzadiHealthComponent> It; It; ++It)
	{
		UAzadiHealthComponent* Health = *It;
		if (!Health || Health->GetOwner() == Owner || Health->IsDead())
		{
			continue;
		}
		AActor* Target = Health->GetOwner();
		if (!Target || Target->GetWorld() != GetWorld())
		{
			continue;
		}
		const float Distance = FVector::Dist(Center, Target->GetActorLocation());
		if (Distance > Def.SplashRadius)
		{
			continue;
		}

		UGameplayStatics::ApplyDamage(Target, Def.Damage, OwnerController, GetOwner(), nullptr);
		if (ACharacter* TargetCharacter = Cast<ACharacter>(Target))
		{
			FVector Away = (Target->GetActorLocation() - Center).GetSafeNormal2D();
			TargetCharacter->LaunchCharacter(Away * Def.ChantImpulse + FVector(0, 0, Def.ChantImpulse * 0.4f), true, true);
		}
		bAnyHit = true;
	}
	if (bAnyHit)
	{
		RegisterHit();
	}
}

void UAzadiWeaponComponent::DoProjectile(const FAzadiWeaponDef& Def)
{
	FVector ViewLocation;
	FRotator ViewRotation;
	GetAimView(ViewLocation, ViewRotation);

	const FVector SpawnLocation = ViewLocation + ViewRotation.Vector() * 100.f + FVector(0, 0, -20.f);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AAzadiProjectile* Projectile = GetWorld()->SpawnActor<AAzadiProjectile>(
		AAzadiProjectile::StaticClass(), SpawnLocation, ViewRotation, SpawnParams);
	if (Projectile)
	{
		Projectile->InitProjectile(ViewRotation.Vector(), Def.ProjectileSpeed, Def.Damage,
			Def.SplashRadius, Def.SplashDamage, 1.f, FLinearColor(1.f, 0.45f, 0.1f));
	}
}

void UAzadiWeaponComponent::GetAimView(FVector& OutLocation, FRotator& OutRotation) const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn)
	{
		if (const AController* Controller = OwnerPawn->GetController())
		{
			Controller->GetPlayerViewPoint(OutLocation, OutRotation);
			return;
		}
	}
	if (const AActor* Owner = GetOwner())
	{
		Owner->GetActorEyesViewPoint(OutLocation, OutRotation);
	}
}

void UAzadiWeaponComponent::ApplyRecoil(const FAzadiWeaponDef& Def) const
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (AController* Controller = OwnerPawn ? OwnerPawn->GetController() : nullptr)
	{
		const float Kick = Def.RecoilPitch * FMath::Lerp(1.f, 0.55f, AdsAlpha);
		Controller->SetControlRotation(Controller->GetControlRotation() + FRotator(Kick, FMath::FRandRange(-Kick, Kick) * 0.25f, 0.f));
	}
}

void UAzadiWeaponComponent::RegisterHit()
{
	LastHitTime = GetWorld()->GetTimeSeconds();
}
