// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKartMovementCompoment.h"


// Sets default values for this component's properties
UGoKartMovementCompoment::UGoKartMovementCompoment()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called every frame
void UGoKartMovementCompoment::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwnerRole() == ROLE_AutonomousProxy || GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		LastMove = CreateMove(DeltaTime);
		SimulateMove(LastMove);
	}
}


void UGoKartMovementCompoment::SimulateMove(const FGoKartMove& Move)
{
	// f = ma
	FVector Force = GetOwner()->GetActorForwardVector() * MaxDrivingForce * Move.Throttle;
	Force += GetAirResistance();
	Force += GetRollingResistance();

	// a = f/m (eqation from f = ma)
	FVector Acceleration = Force / Mass;
	Velocity = Velocity + Acceleration * Move.DeltaTime;

	// convert to m to cm
	FVector	Translation = Velocity * Move.DeltaTime * 100.0f;

	// simulate movement
	ApplyRotation(Move.DeltaTime, Move.SteeringThrow);
	UpdateLocationFromVelocity(Move.DeltaTime);
}


FGoKartMove UGoKartMovementCompoment::CreateMove(float DeltaTime) const
{
	FGoKartMove NewMove;

	NewMove.DeltaTime = DeltaTime;
	NewMove.SteeringThrow = SteeringThrow;
	NewMove.Throttle = Throttle;
	NewMove.Time = GetWorld()->TimeSeconds;

	return NewMove;
}


void UGoKartMovementCompoment::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity * 100 * DeltaTime;
	FHitResult Hit;

	GetOwner()->AddActorWorldOffset(Translation, true, OUT &Hit);

	if (Hit.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}


void UGoKartMovementCompoment::ApplyRotation(float DeltaTime, float SteeringThrow)
{
	float DeltaLocation = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationAngle = DeltaLocation / MinTurningRadius * SteeringThrow;
	FQuat RotationDelta(GetOwner()->GetActorUpVector(), RotationAngle);

	Velocity = RotationDelta.RotateVector(Velocity);

	GetOwner()->AddActorWorldRotation(RotationDelta);
}
