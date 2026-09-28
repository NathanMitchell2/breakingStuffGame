// Fill out your copyright notice in the Description page of Project Settings.


#include "CppDoor.h"

// Sets default values
ACppDoor::ACppDoor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	DoorMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ACppDoor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACppDoor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ShouldOpenOrClose) {
		FRotator CurrentRotation = GetActorRotation();
		FRotator NewRotation;
		NewRotation.Roll = CurrentRotation.Roll;
		NewRotation.Pitch = CurrentRotation.Pitch;
		NewRotation.Yaw = CurrentRotation.Yaw + YawIncrement;
		SetActorRotation(NewRotation);

		if ((YawIncrement > 0 && NewRotation.Yaw >= 90) || (YawIncrement < 0 && NewRotation.Yaw <= 0)) {
			ShouldOpenOrClose = false;
		}
	}



}

void ACppDoor::OpenDoor() {
	if (GetActorRotation().Yaw < 90) {
		YawIncrement = 2.0f;
		ShouldOpenOrClose = true;
	}
}

void ACppDoor::CloseDoor() {
	if (GetActorRotation().Yaw >= 90) {
		YawIncrement = -2.0f;
		ShouldOpenOrClose = true;
	}
}

