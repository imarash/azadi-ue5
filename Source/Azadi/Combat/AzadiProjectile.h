// AZADI: Rise of the Dawn

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AzadiProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;

/**
 * Shared projectile for player launchers and ranged enemies (wraith bolts,
 * molotov arcs). All tuning comes from weapon/enemy defs via InitProjectile.
 */
UCLASS()
class AZADI_API AAzadiProjectile : public AActor
{
	GENERATED_BODY()

public:
	AAzadiProjectile();

	/**
	 * @param Direction      normalized launch direction
	 * @param Speed          initial speed (cm/s)
	 * @param InDamage       direct-hit damage
	 * @param InSplashRadius radial damage radius (0 = direct only)
	 * @param InSplashDamage radial damage at epicenter
	 * @param GravityScale   0 = straight bolt, 1 = ballistic arc
	 * @param Tint           placeholder visual tint
	 */
	void InitProjectile(const FVector& Direction, float Speed, float InDamage, float InSplashRadius,
		float InSplashDamage, float GravityScale, const FLinearColor& Tint);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, Category = "Azadi")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, Category = "Azadi")
	TObjectPtr<UStaticMeshComponent> Visual;

	UPROPERTY(VisibleAnywhere, Category = "Azadi")
	TObjectPtr<UProjectileMovementComponent> Movement;

private:
	float Damage = 10.f;
	float SplashRadius = 0.f;
	float SplashDamage = 0.f;
	bool bExploded = false;
};
