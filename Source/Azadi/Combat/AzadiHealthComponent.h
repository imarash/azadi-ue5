// AZADI: Rise of the Dawn

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AzadiHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAzadiHealthChanged, float, NewHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAzadiDied, AActor*, Killer);

/** Team ids: 0 = the people (player, citizens), 1 = the regime. */
UCLASS(ClassGroup = (Azadi), meta = (BlueprintSpawnableComponent))
class AZADI_API UAzadiHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAzadiHealthComponent();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azadi")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Azadi")
	uint8 TeamId = 0;

	UFUNCTION(BlueprintPure, Category = "Azadi")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Azadi")
	float GetHealthFraction() const { return MaxHealth > 0.f ? Health / MaxHealth : 0.f; }

	UFUNCTION(BlueprintPure, Category = "Azadi")
	bool IsDead() const { return Health <= 0.f; }

	/** Reset pool (used after data-driven init). */
	void InitHealth(float NewMax);

	/** World seconds of the last damage taken — drives HUD hit flash. */
	float GetLastDamageTime() const { return LastDamageTime; }

	UPROPERTY(BlueprintAssignable, Category = "Azadi")
	FAzadiHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Azadi")
	FAzadiDied OnDied;

private:
	UFUNCTION()
	void HandleAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
		AController* InstigatedBy, AActor* DamageCauser);

	float Health = 100.f;
	float LastDamageTime = -1000.f;
	bool bDiedBroadcast = false;
};
