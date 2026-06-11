// AZADI: Rise of the Dawn

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AzadiCharacter.generated.h"

class AAzadiCitizenCage;
class UAzadiHealthComponent;
class UAzadiWeaponComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UStaticMeshComponent;
struct FInputActionValue;

/**
 * First-person freedom fighter. Input is built in code (runtime Enhanced
 * Input) so the project stays text-only; combat is delegated to the
 * data-driven weapon component.
 */
UCLASS()
class AZADI_API AAzadiCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAzadiCharacter();

	virtual void Tick(float DeltaSeconds) override;

	UAzadiWeaponComponent* GetWeapon() const { return Weapon; }
	UAzadiHealthComponent* GetHealth() const { return Health; }

	/** Citizen cage under the crosshair within interact range, if any. */
	AAzadiCitizenCage* FindInteractable() const;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_JumpStart(const FInputActionValue& Value);
	void Input_JumpStop(const FInputActionValue& Value);
	void Input_SprintStart(const FInputActionValue& Value);
	void Input_SprintStop(const FInputActionValue& Value);
	void Input_FireStart(const FInputActionValue& Value);
	void Input_FireStop(const FInputActionValue& Value);
	void Input_AdsStart(const FInputActionValue& Value);
	void Input_AdsStop(const FInputActionValue& Value);
	void Input_Reload(const FInputActionValue& Value);
	void Input_Interact(const FInputActionValue& Value);
	void Input_Cycle(const FInputActionValue& Value);
	void Input_Slot1(const FInputActionValue& Value);
	void Input_Slot2(const FInputActionValue& Value);
	void Input_Slot3(const FInputActionValue& Value);

	UFUNCTION()
	void HandleDied(AActor* Killer);

private:
	void BuildInputMappings();
	void UpdateMovementSpeed();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UCameraComponent> Camera;

	/** Placeholder first-person weapon block; readable ADS without assets. */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ViewModel;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UAzadiHealthComponent> Health;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UAzadiWeaponComponent> Weapon;

	// Runtime-built input objects (kept alive via UPROPERTY).
	UPROPERTY(Transient) TObjectPtr<UInputMappingContext> InputContext;
	UPROPERTY(Transient) TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> LookAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> SprintAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> FireAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> AdsAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> ReloadAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> InteractAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> CycleAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> Slot1Action;
	UPROPERTY(Transient) TObjectPtr<UInputAction> Slot2Action;
	UPROPERTY(Transient) TObjectPtr<UInputAction> Slot3Action;

	float HipFov = 90.f;
	float WalkSpeed = 460.f;
	float SprintSpeed = 640.f;
	float AdsSpeedMul = 0.55f;
	bool bSprinting = false;
};
