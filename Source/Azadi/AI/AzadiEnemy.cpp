// AZADI: Rise of the Dawn

#include "AI/AzadiEnemy.h"
#include "AI/AzadiEnemyController.h"
#include "Azadi.h"
#include "Combat/AzadiHealthComponent.h"
#include "Combat/AzadiProjectile.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/AzadiGameMode.h"
#include "Core/AzadiLiberationSubsystem.h"
#include "Core/AzadiModuleRegistry.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AAzadiEnemy::AAzadiEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(36.f, 90.f);

	AIControllerClass = AAzadiEnemyController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 480.f, 0.f);

	Health = CreateDefaultSubobject<UAzadiHealthComponent>(TEXT("Health"));
	Health->TeamId = 1;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(GetCapsuleComponent());
	Body->SetCollisionProfileName(TEXT("NoCollision"));
	Body->SetRelativeLocation(FVector(0.f, 0.f, -20.f));
	Body->SetRelativeScale3D(FVector(0.55f, 0.75f, 1.35f));

	Hood = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hood"));
	Hood->SetupAttachment(GetCapsuleComponent());
	Hood->SetCollisionProfileName(TEXT("NoCollision"));
	Hood->SetRelativeLocation(FVector(0.f, 0.f, 78.f));
	Hood->SetRelativeScale3D(FVector(0.55f, 0.55f, 0.6f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (CubeMesh.Succeeded())
	{
		Body->SetStaticMesh(CubeMesh.Object);
	}
	if (ConeMesh.Succeeded())
	{
		Hood->SetStaticMesh(ConeMesh.Object);
	}
}

void AAzadiEnemy::BeginPlay()
{
	Super::BeginPlay();
	Health->OnDied.AddDynamic(this, &AAzadiEnemy::HandleDied);

	if (AAzadiGameMode* GameMode = GetWorld()->GetAuthGameMode<AAzadiGameMode>())
	{
		GameMode->RegisterEnemySpawned(this);
	}
}

void AAzadiEnemy::InitFromDef(const FAzadiEnemyDef& InDef)
{
	Def = InDef;

	Health->InitHealth(Def.Health);
	GetCharacterMovement()->MaxWalkSpeed = Def.MoveSpeed;

	if (!FMath::IsNearlyEqual(Def.Scale, 1.f))
	{
		SetActorScale3D(FVector(Def.Scale));
	}

	// Optional skinned mesh from an asset/mod pack.
	if (!Def.MeshPath.IsEmpty())
	{
		if (USkeletalMesh* Skinned = LoadObject<USkeletalMesh>(nullptr, *Def.MeshPath))
		{
			GetMesh()->SetSkeletalMesh(Skinned);
			Body->SetVisibility(false);
			Hood->SetVisibility(false);
		}
		else if (UStaticMesh* Static = LoadObject<UStaticMesh>(nullptr, *Def.MeshPath))
		{
			Body->SetStaticMesh(Static);
			Body->SetRelativeScale3D(FVector(1.f));
			Hood->SetVisibility(false);
		}
	}

	ApplyPlaceholderTint();
}

void AAzadiEnemy::ApplyPlaceholderTint()
{
	UMaterialInterface* Solid = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Azadi/Materials/M_AzadiSolid.M_AzadiSolid"));
	if (!Solid)
	{
		return;
	}
	const FLinearColor Tint = UAzadiModuleRegistry::ParseColorHex(Def.ColorHex, FLinearColor(0.2f, 0.2f, 0.25f));
	for (UStaticMeshComponent* Component : { Body.Get(), Hood.Get() })
	{
		if (Component && Component->IsVisible())
		{
			if (UMaterialInstanceDynamic* Mid = Component->CreateDynamicMaterialInstance(0, Solid))
			{
				Mid->SetVectorParameterValue(TEXT("Color"), Tint);
			}
		}
	}
}

void AAzadiEnemy::TryAttack(APawn* Target)
{
	if (bDying || !Target)
	{
		return;
	}
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now < NextAttackTime)
	{
		return;
	}
	NextAttackTime = Now + Def.AttackInterval;

	const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();

	if (Def.AttackKind == EAzadiAttackKind::Melee)
	{
		if (ToTarget.Size() <= Def.AttackRange * 1.25f)
		{
			UGameplayStatics::ApplyDamage(Target, Def.Damage, GetController(), this, nullptr);
		}
	}
	else
	{
		const FVector MuzzleLocation = GetActorLocation() + FVector(0.f, 0.f, 60.f) + ToTarget.GetSafeNormal() * 70.f;
		const FVector AimPoint = Target->GetActorLocation() + FVector(0.f, 0.f, 30.f);
		const FVector Dir = (AimPoint - MuzzleLocation).GetSafeNormal();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AAzadiProjectile* Bolt = GetWorld()->SpawnActor<AAzadiProjectile>(
			AAzadiProjectile::StaticClass(), MuzzleLocation, Dir.Rotation(), SpawnParams);
		if (Bolt)
		{
			Bolt->InitProjectile(Dir, Def.ProjectileSpeed, Def.Damage, 0.f, 0.f, 0.f,
				FLinearColor(0.55f, 0.1f, 0.7f));
		}
	}
}

bool AAzadiEnemy::IsDead() const
{
	return bDying || (Health && Health->IsDead());
}

void AAzadiEnemy::HandleDied(AActor* Killer)
{
	if (bDying)
	{
		return;
	}
	bDying = true;

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();

	if (AController* MyController = GetController())
	{
		MyController->UnPossess();
	}

	UE_LOG(LogAzadi, Log, TEXT("Enemy down: %s (+%.2f AZADI)"), *Def.Id.ToString(), Def.LiberationReward);

	if (UAzadiLiberationSubsystem* Liberation = GetWorld()->GetSubsystem<UAzadiLiberationSubsystem>())
	{
		Liberation->AddLiberation(Def.LiberationReward);
	}
	if (AAzadiGameMode* GameMode = GetWorld()->GetAuthGameMode<AAzadiGameMode>())
	{
		GameMode->NotifyEnemyKilled(this);
	}

	OnEnemyDied.Broadcast(this);
	SetLifeSpan(2.f);
}

void AAzadiEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Placeholder death: sink through the ground while fading lifespan runs out.
	if (bDying)
	{
		DeathSinkSeconds += DeltaSeconds;
		AddActorWorldOffset(FVector(0.f, 0.f, -90.f * DeltaSeconds), false);
		SetActorRotation(GetActorRotation() + FRotator(0.f, 0.f, 140.f * DeltaSeconds));
	}
}
