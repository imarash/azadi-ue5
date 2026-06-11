// AZADI: Rise of the Dawn

#include "Core/AzadiLiberationSubsystem.h"
#include "Azadi.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"

void UAzadiLiberationSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Optional: created by the editor bootstrap script. Soft dependency.
	Mpc = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/Azadi/Materials/MPC_Azadi.MPC_Azadi"));
	PushToMaterials();
}

void UAzadiLiberationSubsystem::AddLiberation(float Amount)
{
	if (Amount <= 0.f || Meter >= 1.f)
	{
		return;
	}

	Meter = FMath::Clamp(Meter + Amount, 0.f, 1.f);
	PushToMaterials();
	OnLiberationChanged.Broadcast(Meter);

	static const float Milestones[] = { 0.25f, 0.5f, 0.75f, 1.f };
	for (int32 i = 0; i < UE_ARRAY_COUNT(Milestones); ++i)
	{
		const uint8 Bit = 1 << i;
		if (Meter >= Milestones[i] && !(MilestonesFired & Bit))
		{
			MilestonesFired |= Bit;
			UE_LOG(LogAzadi, Log, TEXT("Liberation: milestone %.2f reached"), Milestones[i]);
			OnMilestone.Broadcast(Milestones[i]);
		}
	}
}

void UAzadiLiberationSubsystem::ReportCombatant(bool bEngaged)
{
	const bool bWasActive = EngagedCount > 0;
	EngagedCount = FMath::Max(0, EngagedCount + (bEngaged ? 1 : -1));
	const bool bIsActive = EngagedCount > 0;

	if (bWasActive != bIsActive)
	{
		OnCombatStateChanged.Broadcast(bIsActive);
	}
}

void UAzadiLiberationSubsystem::PushToMaterials() const
{
	if (Mpc)
	{
		UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), Mpc, TEXT("Liberation"), Meter);
	}
}
