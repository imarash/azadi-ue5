// AZADI: Rise of the Dawn

#include "Player/AzadiCharacter.h"
#include "Azadi.h"
#include "Camera/CameraComponent.h"
#include "Combat/AzadiHealthComponent.h"
#include "Combat/AzadiWeaponComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/AzadiDistrictSubsystem.h"
#include "Core/AzadiGameMode.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "World/AzadiCitizenCage.h"

AAzadiCharacter::AAzadiCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(34.f, 96.f);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetCapsuleComponent());
	Camera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	Camera->bUsePawnControlRotation = true;
	Camera->SetFieldOfView(HipFov);

	ViewModel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ViewModel"));
	ViewModel->SetupAttachment(Camera);
	ViewModel->SetCollisionProfileName(TEXT("NoCollision"));
	ViewModel->SetRelativeLocation(FVector(42.f, 16.f, -14.f));
	ViewModel->SetRelativeScale3D(FVector(0.5f, 0.055f, 0.07f));
	ViewModel->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		ViewModel->SetStaticMesh(CubeMesh.Object);
	}

	Health = CreateDefaultSubobject<UAzadiHealthComponent>(TEXT("Health"));
	Health->TeamId = 0;
	Health->MaxHealth = 100.f;

	Weapon = CreateDefaultSubobject<UAzadiWeaponComponent>(TEXT("Weapon"));

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->AirControl = 0.4f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.f;
}

void AAzadiCharacter::BeginPlay()
{
	Super::BeginPlay();

	Health->OnDied.AddDynamic(this, &AAzadiCharacter::HandleDied);

	// Loadout comes from the active district (data), with a bare-hands fallback.
	TArray<FName> Loadout = { TEXT("braid_whip"), TEXT("sidearm") };
	if (const UAzadiDistrictSubsystem* District = GetWorld()->GetSubsystem<UAzadiDistrictSubsystem>())
	{
		if (District->HasDistrict() && District->GetDistrict().Loadout.Num() > 0)
		{
			Loadout = District->GetDistrict().Loadout;
		}
	}
	Weapon->InitLoadout(Loadout);

	if (UMaterialInterface* Solid = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Azadi/Materials/M_AzadiSolid.M_AzadiSolid")))
	{
		if (UMaterialInstanceDynamic* Mid = ViewModel->CreateDynamicMaterialInstance(0, Solid))
		{
			Mid->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.05f, 0.05f, 0.06f));
		}
	}
}

void AAzadiCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// ADS: data-driven FOV + viewmodel centering.
	const float TargetFov = Weapon->GetTargetFov(HipFov);
	Camera->SetFieldOfView(FMath::FInterpTo(Camera->FieldOfView, TargetFov, DeltaSeconds, 14.f));

	const float Ads = Weapon->GetAdsAlpha();
	const FVector HipPos(42.f, 16.f, -14.f);
	const FVector AdsPos(38.f, 0.f, -11.f);
	ViewModel->SetRelativeLocation(FMath::Lerp(HipPos, AdsPos, Ads));

	UpdateMovementSpeed();
}

void AAzadiCharacter::UpdateMovementSpeed()
{
	float Speed = bSprinting ? SprintSpeed : WalkSpeed;
	Speed *= FMath::Lerp(1.f, AdsSpeedMul, Weapon->GetAdsAlpha());
	GetCharacterMovement()->MaxWalkSpeed = Speed;
}

void AAzadiCharacter::BuildInputMappings()
{
	InputContext = NewObject<UInputMappingContext>(this, TEXT("IMC_Azadi"));

	auto MakeAction = [this](FName Name, EInputActionValueType ValueType)
	{
		UInputAction* Action = NewObject<UInputAction>(this, Name);
		Action->ValueType = ValueType;
		return Action;
	};
	auto Negate = [this]() { return NewObject<UInputModifierNegate>(this); };
	auto SwizzleYXZ = [this]() { return NewObject<UInputModifierSwizzleAxis>(this); }; // default order YXZ

	// Move: WASD + left stick -> Axis2D (X = right, Y = forward)
	MoveAction = MakeAction(TEXT("IA_Move"), EInputActionValueType::Axis2D);
	InputContext->MapKey(MoveAction, EKeys::W).Modifiers.Add(SwizzleYXZ());
	{
		FEnhancedActionKeyMapping& M = InputContext->MapKey(MoveAction, EKeys::S);
		M.Modifiers.Add(Negate());
		M.Modifiers.Add(SwizzleYXZ());
	}
	InputContext->MapKey(MoveAction, EKeys::D);
	InputContext->MapKey(MoveAction, EKeys::A).Modifiers.Add(Negate());
	InputContext->MapKey(MoveAction, EKeys::Gamepad_LeftX);
	InputContext->MapKey(MoveAction, EKeys::Gamepad_LeftY).Modifiers.Add(SwizzleYXZ());

	// Look: mouse + right stick
	LookAction = MakeAction(TEXT("IA_Look"), EInputActionValueType::Axis2D);
	InputContext->MapKey(LookAction, EKeys::Mouse2D);
	InputContext->MapKey(LookAction, EKeys::Gamepad_RightX);
	InputContext->MapKey(LookAction, EKeys::Gamepad_RightY).Modifiers.Add(SwizzleYXZ());

	JumpAction = MakeAction(TEXT("IA_Jump"), EInputActionValueType::Boolean);
	InputContext->MapKey(JumpAction, EKeys::SpaceBar);
	InputContext->MapKey(JumpAction, EKeys::Gamepad_FaceButton_Bottom);

	SprintAction = MakeAction(TEXT("IA_Sprint"), EInputActionValueType::Boolean);
	InputContext->MapKey(SprintAction, EKeys::LeftShift);
	InputContext->MapKey(SprintAction, EKeys::Gamepad_LeftThumbstick);

	FireAction = MakeAction(TEXT("IA_Fire"), EInputActionValueType::Boolean);
	InputContext->MapKey(FireAction, EKeys::LeftMouseButton);
	InputContext->MapKey(FireAction, EKeys::Gamepad_RightTrigger);

	AdsAction = MakeAction(TEXT("IA_Ads"), EInputActionValueType::Boolean);
	InputContext->MapKey(AdsAction, EKeys::RightMouseButton);
	InputContext->MapKey(AdsAction, EKeys::Gamepad_LeftTrigger);

	ReloadAction = MakeAction(TEXT("IA_Reload"), EInputActionValueType::Boolean);
	InputContext->MapKey(ReloadAction, EKeys::R);
	InputContext->MapKey(ReloadAction, EKeys::Gamepad_FaceButton_Left);

	InteractAction = MakeAction(TEXT("IA_Interact"), EInputActionValueType::Boolean);
	InputContext->MapKey(InteractAction, EKeys::E);
	InputContext->MapKey(InteractAction, EKeys::Gamepad_FaceButton_Top);

	CycleAction = MakeAction(TEXT("IA_Cycle"), EInputActionValueType::Axis1D);
	InputContext->MapKey(CycleAction, EKeys::MouseWheelAxis);
	InputContext->MapKey(CycleAction, EKeys::Gamepad_RightShoulder);
	InputContext->MapKey(CycleAction, EKeys::Gamepad_LeftShoulder).Modifiers.Add(Negate());
	InputContext->MapKey(CycleAction, EKeys::Q).Modifiers.Add(Negate());

	Slot1Action = MakeAction(TEXT("IA_Slot1"), EInputActionValueType::Boolean);
	InputContext->MapKey(Slot1Action, EKeys::One);
	Slot2Action = MakeAction(TEXT("IA_Slot2"), EInputActionValueType::Boolean);
	InputContext->MapKey(Slot2Action, EKeys::Two);
	Slot3Action = MakeAction(TEXT("IA_Slot3"), EInputActionValueType::Boolean);
	InputContext->MapKey(Slot3Action, EKeys::Three);
}

void AAzadiCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	BuildInputMappings();

	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(InputContext, 0);
		}
	}

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input)
	{
		UE_LOG(LogAzadi, Error, TEXT("Character: EnhancedInputComponent required (check DefaultInput.ini)"));
		return;
	}

	Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAzadiCharacter::Input_Move);
	Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAzadiCharacter::Input_Look);
	Input->BindAction(JumpAction, ETriggerEvent::Started, this, &AAzadiCharacter::Input_JumpStart);
	Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &AAzadiCharacter::Input_JumpStop);
	Input->BindAction(SprintAction, ETriggerEvent::Started, this, &AAzadiCharacter::Input_SprintStart);
	Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &AAzadiCharacter::Input_SprintStop);
	Input->BindAction(FireAction, ETriggerEvent::Started, this, &AAzadiCharacter::Input_FireStart);
	Input->BindAction(FireAction, ETriggerEvent::Completed, this, &AAzadiCharacter::Input_FireStop);
	Input->BindAction(AdsAction, ETriggerEvent::Started, this, &AAzadiCharacter::Input_AdsStart);
	Input->BindAction(AdsAction, ETriggerEvent::Completed, this, &AAzadiCharacter::Input_AdsStop);
	Input->BindAction(ReloadAction, ETriggerEvent::Started, this, &AAzadiCharacter::Input_Reload);
	Input->BindAction(InteractAction, ETriggerEvent::Started, this, &AAzadiCharacter::Input_Interact);
	Input->BindAction(CycleAction, ETriggerEvent::Started, this, &AAzadiCharacter::Input_Cycle);
	Input->BindAction(Slot1Action, ETriggerEvent::Started, this, &AAzadiCharacter::Input_Slot1);
	Input->BindAction(Slot2Action, ETriggerEvent::Started, this, &AAzadiCharacter::Input_Slot2);
	Input->BindAction(Slot3Action, ETriggerEvent::Started, this, &AAzadiCharacter::Input_Slot3);
}

void AAzadiCharacter::Input_Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddMovementInput(GetActorRightVector(), Axis.X);
	AddMovementInput(GetActorForwardVector(), Axis.Y);
}

void AAzadiCharacter::Input_Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	AddControllerPitchInput(-Axis.Y);
}

void AAzadiCharacter::Input_JumpStart(const FInputActionValue& Value) { Jump(); }
void AAzadiCharacter::Input_JumpStop(const FInputActionValue& Value) { StopJumping(); }
void AAzadiCharacter::Input_SprintStart(const FInputActionValue& Value) { bSprinting = true; }
void AAzadiCharacter::Input_SprintStop(const FInputActionValue& Value) { bSprinting = false; }
void AAzadiCharacter::Input_FireStart(const FInputActionValue& Value) { Weapon->StartFire(); }
void AAzadiCharacter::Input_FireStop(const FInputActionValue& Value) { Weapon->StopFire(); }
void AAzadiCharacter::Input_AdsStart(const FInputActionValue& Value) { Weapon->SetAds(true); }
void AAzadiCharacter::Input_AdsStop(const FInputActionValue& Value) { Weapon->SetAds(false); }
void AAzadiCharacter::Input_Reload(const FInputActionValue& Value) { Weapon->StartReload(); }
void AAzadiCharacter::Input_Slot1(const FInputActionValue& Value) { Weapon->EquipIndex(0); }
void AAzadiCharacter::Input_Slot2(const FInputActionValue& Value) { Weapon->EquipIndex(1); }
void AAzadiCharacter::Input_Slot3(const FInputActionValue& Value) { Weapon->EquipIndex(2); }

void AAzadiCharacter::Input_Cycle(const FInputActionValue& Value)
{
	Weapon->CycleWeapon(Value.Get<float>() > 0.f ? 1 : -1);
}

void AAzadiCharacter::Input_Interact(const FInputActionValue& Value)
{
	if (AAzadiCitizenCage* Cage = FindInteractable())
	{
		Cage->TryFree(this);
	}
}

AAzadiCitizenCage* AAzadiCharacter::FindInteractable() const
{
	FVector ViewLocation;
	FRotator ViewRotation;
	if (const AController* MyController = GetController())
	{
		MyController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}
	else
	{
		GetActorEyesViewPoint(ViewLocation, ViewRotation);
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(AzadiInteract), false, this);
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, ViewLocation,
		ViewLocation + ViewRotation.Vector() * 420.f, ECC_Visibility, Params))
	{
		return Cast<AAzadiCitizenCage>(Hit.GetActor());
	}
	return nullptr;
}

void AAzadiCharacter::HandleDied(AActor* Killer)
{
	Weapon->StopFire();
	Weapon->SetAds(false);
	DisableInput(Cast<APlayerController>(GetController()));

	if (AAzadiGameMode* GameMode = GetWorld()->GetAuthGameMode<AAzadiGameMode>())
	{
		GameMode->NotifyPlayerDied(GetController());
	}
}
