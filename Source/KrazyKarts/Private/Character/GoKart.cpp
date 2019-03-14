// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"


// Sets default values
AGoKart::AGoKart()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bReplicateMovement = false;

	MovementComponent = CreateDefaultSubobject<UGoKartMovementCompoment>(TEXT("Movement Component"));
	MovementReplicator = CreateDefaultSubobject<UGoKartMovementReplicator>(TEXT("Movement Replicator"));
}


// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		NetUpdateFrequency = 10;
	}
}


// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);
}


void AGoKart::MoveForward(float Axis)
{
	if (MovementComponent)
	{
		MovementComponent->SetThrottle(Axis);
	}
}


void AGoKart::MoveRight(float Axis)
{
	if (MovementComponent)
	{
		MovementComponent->SetSteeringThrow(Axis);
	}
}
