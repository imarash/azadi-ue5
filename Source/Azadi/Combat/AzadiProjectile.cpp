// AZADI: Rise of the Dawn

#include "Combat/AzadiProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AAzadiProjectile::AAzadiProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	InitialLifeSpan = 8.f;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(7.f);
	Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	Collision->SetNotifyRigidBodyCollision(true);
	RootComponent = Collision;

	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(Collision);
	Visual->SetCollisionProfileName(TEXT("NoCollision"));
	Visual->SetRelativeScale3D(FVector(0.14f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		Visual->SetStaticMesh(SphereMesh.Object);
	}

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->UpdatedComponent = Collision;
	Movement->bRotationFollowsVelocity = true;
	Movement->bShouldBounce = false;
	Movement->ProjectileGravityScale = 0.f;
}

void AAzadiProjectile::BeginPlay()
{
	Super::BeginPlay();
	Collision->OnComponentHit.AddDynamic(this, &AAzadiProjectile::OnHit);

	// Never collide with whoever fired us.
	if (AActor* MyOwner = GetOwner())
	{
		Collision->IgnoreActorWhenMoving(MyOwner, true);
	}
	if (APawn* MyInstigator = GetInstigator())
	{
		Collision->IgnoreActorWhenMoving(MyInstigator, true);
		MyInstigator->MoveIgnoreActorAdd(this);
	}
}

void AAzadiProjectile::InitProjectile(const FVector& Direction, float Speed, float InDamage, float InSplashRadius,
	float InSplashDamage, float GravityScale, const FLinearColor& Tint)
{
	Damage = InDamage;
	SplashRadius = InSplashRadius;
	SplashDamage = InSplashDamage;

	Movement->ProjectileGravityScale = GravityScale;
	Movement->InitialSpeed = Speed;
	Movement->MaxSpeed = Speed * 1.5f;
	Movement->Velocity = Direction.GetSafeNormal() * Speed;

	if (UMaterialInterface* Solid = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Azadi/Materials/M_AzadiSolid.M_AzadiSolid")))
	{
		UMaterialInstanceDynamic* Mid = Visual->CreateDynamicMaterialInstance(0, Solid);
		if (Mid)
		{
			Mid->SetVectorParameterValue(TEXT("Color"), Tint);
			Mid->SetScalarParameterValue(TEXT("Emissive"), 8.f);
		}
	}
}

void AAzadiProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (bExploded)
	{
		return;
	}
	bExploded = true;

	AController* InstigatorController = GetInstigatorController();

	if (SplashRadius > 1.f)
	{
		TArray<AActor*> Ignore;
		Ignore.Add(this);
		UGameplayStatics::ApplyRadialDamageWithFalloff(this, SplashDamage, SplashDamage * 0.25f,
			GetActorLocation(), SplashRadius * 0.3f, SplashRadius, 1.f,
			nullptr, Ignore, this, InstigatorController);
	}

	if (OtherActor && OtherActor != this)
	{
		UGameplayStatics::ApplyPointDamage(OtherActor, Damage, GetActorForwardVector(), Hit,
			InstigatorController, this, nullptr);
	}

	Destroy();
}
