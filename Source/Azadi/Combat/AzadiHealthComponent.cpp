// AZADI: Rise of the Dawn

#include "Combat/AzadiHealthComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

UAzadiHealthComponent::UAzadiHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAzadiHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UAzadiHealthComponent::HandleAnyDamage);
	}
}

void UAzadiHealthComponent::InitHealth(float NewMax)
{
	MaxHealth = NewMax;
	Health = NewMax;
	bDiedBroadcast = false;
	OnHealthChanged.Broadcast(Health, MaxHealth);
}

void UAzadiHealthComponent::HandleAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.f || IsDead())
	{
		return;
	}

	// No friendly fire between same-team actors.
	if (InstigatedBy)
	{
		if (const APawn* InstigatorPawn = InstigatedBy->GetPawn())
		{
			if (const UAzadiHealthComponent* Other = InstigatorPawn->FindComponentByClass<UAzadiHealthComponent>())
			{
				if (Other != this && Other->TeamId == TeamId)
				{
					return;
				}
			}
		}
	}

	Health = FMath::Max(0.f, Health - Damage);
	LastDamageTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	OnHealthChanged.Broadcast(Health, MaxHealth);

	if (Health <= 0.f && !bDiedBroadcast)
	{
		bDiedBroadcast = true;
		AActor* Killer = InstigatedBy ? Cast<AActor>(InstigatedBy->GetPawn()) : DamageCauser;
		OnDied.Broadcast(Killer);
	}
}
