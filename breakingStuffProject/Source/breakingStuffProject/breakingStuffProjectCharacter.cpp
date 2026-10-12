// Copyright Epic Games, Inc. All Rights Reserved.

#include "breakingStuffProjectCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "breakingStuffProject.h"

AbreakingStuffProjectCharacter::AbreakingStuffProjectCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
	
	AttackCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Attack Collision"));
	AttackCollision->SetupAttachment(GetMesh(), FName("hand_r"));
	AttackCollision->SetSphereRadius(40.0);
	AttackCollision->SetHiddenInGame(true);
	AttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackCollision->OnComponentBeginOverlap.AddDynamic(this, &AbreakingStuffProjectCharacter::OnBallHit);
}

void AbreakingStuffProjectCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AttackMontage && AnimInstance && AnimInstance->Montage_IsPlaying(AttackMontage))
	{
		AttackCollision->SetWorldRotation(GetMesh()->GetSocketRotation(FName("hand_r")));
	}
}

void AbreakingStuffProjectCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AbreakingStuffProjectCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AbreakingStuffProjectCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AbreakingStuffProjectCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AbreakingStuffProjectCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AbreakingStuffProjectCharacter::LookInput);

		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AbreakingStuffProjectCharacter::Attack);
	}
	else
	{
		UE_LOG(LogbreakingStuffProject, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}




void AbreakingStuffProjectCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void AbreakingStuffProjectCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void AbreakingStuffProjectCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AbreakingStuffProjectCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AbreakingStuffProjectCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void AbreakingStuffProjectCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}


void AbreakingStuffProjectCharacter::Attack(const FInputActionValue& Value)
{
	AttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AttackCollision->SetHiddenInGame(false);
	PlayAttackMontage(AttackMontage);
	GetWorldTimerManager().SetTimer(
		AttackCollisionTimerHandle,
		this,
		&AbreakingStuffProjectCharacter::DisableAttackCollision,
		1.0f,
		false
	);
	
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,        
			2.0f,    
			FColor::Green,
			TEXT("Attack button pressed!")
		);
	}
}

void AbreakingStuffProjectCharacter::DisableAttackCollision()
{
	AttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackCollision->SetHiddenInGame(true);
}

void AbreakingStuffProjectCharacter::OnBallHit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherComp && OtherComp->IsA<USphereComponent>())
	{
		TArray<UMeshComponent*> MeshComponents;
		OtherActor->GetComponents<UMeshComponent>(MeshComponents);

		for (UMeshComponent* MeshComponent : MeshComponents)
		{
			if (MeshComponent && MeshComponent->IsSimulatingPhysics())
			{
				FVector HitDirection = AttackCollision->GetForwardVector();

				MeshComponent->AddImpulse(HitDirection * 1000.0f, NAME_None, true);
				break;
			}
		}

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,        
				2.0f,      
				FColor::Red,
				TEXT("Attack collision hit something!")
			);
		}
	}
}
void AbreakingStuffProjectCharacter::PlayAttackMontage(UAnimMontage* MontageToPlay)
 {
	if (MontageToPlay && GetMesh())
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			// Optional: Ensure we aren't already playing a montage to prevent spamming
			if (!AnimInstance->IsAnyMontagePlaying())
			{
				AnimInstance->Montage_Play(MontageToPlay, 1.0f);
			}
		}
	}
 }