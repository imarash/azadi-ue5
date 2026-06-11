// AZADI: Rise of the Dawn

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Data/AzadiTypes.h"
#include "AzadiEnemy.generated.h"

class UAzadiHealthComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAzadiEnemyDied, AAzadiEnemy*, Enemy);

/**
 * Data-driven regime enforcer. One C++ class covers every enemy type:
 * behavior, stats, scale, and tint come from FAzadiEnemyDef. The placeholder
 * body (block + hood) is replaced by MeshPath once art packs land.
 */
UCLASS()
class AZADI_API AAzadiEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AAzadiEnemy();

	void InitFromDef(const FAzadiEnemyDef& InDef);
	const FAzadiEnemyDef& GetDef() const { return Def; }

	/** Attack the target if the def's cooldown allows. */
	void TryAttack(APawn* Target);

	bool IsDead() const;

	UPROPERTY(BlueprintAssignable, Category = "Azadi")
	FAzadiEnemyDied OnEnemyDied;

	UAzadiHealthComponent* GetHealth() const { return Health; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void HandleDied(AActor* Killer);

	UPROPERTY(VisibleAnywhere, Category = "Azadi")
	TObjectPtr<UAzadiHealthComponent> Health;

	/** Placeholder silhouette: torso block + hood cone, tinted per def. */
	UPROPERTY(VisibleAnywhere, Category = "Azadi")
	TObjectPtr<UStaticMeshComponent> Body;

	UPROPERTY(VisibleAnywhere, Category = "Azadi")
	TObjectPtr<UStaticMeshComponent> Hood;

private:
	void ApplyPlaceholderTint();

	FAzadiEnemyDef Def;
	float NextAttackTime = 0.f;
	float DeathSinkSeconds = 0.f;
	bool bDying = false;
};
