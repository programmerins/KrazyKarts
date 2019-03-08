// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"


// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}


// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// f = ma -> a = f/m
	FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;

	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / Mass;

	Velocity = Velocity + Acceleration * DeltaTime;

	FVector	Translation = Velocity * DeltaTime * 100.0f;

	ApplyRotation(DeltaTime);
	UpdateLocationFromVelocity(Translation);	
}


// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);
}


void AGoKart::UpdateLocationFromVelocity(const FVector &Translation)
{
	FHitResult Hit;
	AddActorWorldOffset(Translation, true, OUT &Hit);

	if (Hit.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}


void AGoKart::ApplyRotation(float DeltaTime)
{
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationAngle = DeltaLocation / MinTurningRadius * SteeringThrow;
	FQuat RotationDelta(GetActorUpVector(), RotationAngle);

	Velocity = RotationDelta.RotateVector(Velocity);

	AddActorWorldRotation(RotationDelta);
}


void AGoKart::MoveForward(float Axis)
{
	Throttle = Axis;
}


void AGoKart::Server_MoveForward_Implementation(float Axis)
{

}


bool AGoKart::Server_MoveForward_Validate(float Axis)
{
	return true;
}


void AGoKart::MoveRight(float Axis)
{
	SteeringThrow = Axis;
}


FVector AGoKart::GetAirResistance()
{
	return -Velocity.GetSafeNormal() *  Velocity.SizeSquared() * DragCoefficient;
}


FVector AGoKart::GetRollingResistance()
{
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() * 0.01;
	float NormalForce = Mass * AccelerationDueToGravity;

	return -Velocity.GetSafeNormal() * NormalForce * RollingResistanceCoefficient;
}
