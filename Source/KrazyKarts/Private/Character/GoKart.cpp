// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"


#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"


namespace InsTestSpace
{
	FString GetEnumText(ENetRole Role)
	{
		switch (Role)
		{
		case ROLE_None:
			return "None";

		case ROLE_Authority:
			return "Authority";

		case ROLE_SimulatedProxy:
			return "Simulated Proxy";

		case ROLE_AutonomousProxy:
			return "Autonomous Proxy";

		default:
			return "Error";
		}
	}
}


// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
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

	if (Role == ROLE_AutonomousProxy)
	{
		FGoKartMove Move = CreateMove(DeltaTime);
		SimulateMove(Move);

		UnacknowledgedMoves.Add(Move);
		Server_SendMove(Move);
	}

	if (Role == ROLE_SimulatedProxy)
	{
		SimulateMove(ServerState.LastMove);
	}

	// We are the server and in control of the pawn
	if (Role == ROLE_Authority && GetRemoteRole() == ROLE_SimulatedProxy)
	{
		FGoKartMove Move = CreateMove(DeltaTime);
		Server_SendMove(Move);
	}

	DrawDebugString(GetWorld(), FVector::UpVector * -20.0f, InsTestSpace::GetEnumText(Role), this, FColor::White, 0);
	DrawDebugString(GetWorld(), FVector::ZeroVector, FString::Printf(TEXT("Remote Role = %s"), *InsTestSpace::GetEnumText(GetRemoteRole())), this, FColor::Red, 0);
}


// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);
}


void AGoKart::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity * 100 * DeltaTime;

	FHitResult Hit;
	AddActorWorldOffset(Translation, true, OUT &Hit);

	if (Hit.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}


void AGoKart::ApplyRotation(float DeltaTime, float SteeringThrow)
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


void AGoKart::MoveRight(float Axis)
{
	SteeringThrow = Axis;
}


// locally (client & server)
FGoKartMove AGoKart::CreateMove(float DeltaTime) const
{
	FGoKartMove NewMove;
	NewMove.DeltaTime = DeltaTime;
	NewMove.SteeringThrow = SteeringThrow;
	NewMove.Throttle = Throttle;
	NewMove.Time = GetWorld()->TimeSeconds;

	return NewMove;
}


// only clients notified
void AGoKart::OnRep_ServerState()
{
	SetActorTransform(ServerState.Transform);
	Velocity = ServerState.Velocity;

	ClearAcknowledgedMoves(ServerState.LastMove);

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		SimulateMove(Move);
	}
}


// only client
void AGoKart::ClearAcknowledgedMoves(const FGoKartMove& LastMove)
{
	TArray<FGoKartMove> NewMoves;

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		if (Move.Time > LastMove.Time)
		{
			NewMoves.Add(Move);
		}
	}

	UnacknowledgedMoves = NewMoves;
}


// only server
void AGoKart::Server_SendMove_Implementation(FGoKartMove Move)
{
	SimulateMove(Move);

	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = Velocity;
}


// only server
bool AGoKart::Server_SendMove_Validate(FGoKartMove Move)
{
	return true; // TODO  make better validation
}


// only server
void AGoKart::SimulateMove(FGoKartMove Move)
{
	// f = ma -> a = f/m
	FVector Force = GetActorForwardVector() * MaxDrivingForce * Move.Throttle;

	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / Mass;

	Velocity = Velocity + Acceleration * Move.DeltaTime;

	FVector	Translation = Velocity * Move.DeltaTime * 100.0f;

	ApplyRotation(Move.DeltaTime, Move.SteeringThrow);

	UpdateLocationFromVelocity(Move.DeltaTime);
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


void AGoKart::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGoKart, ServerState);
}
