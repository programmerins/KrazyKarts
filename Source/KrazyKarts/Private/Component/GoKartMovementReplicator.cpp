// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKartMovementReplicator.h"

#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"


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


// Sets default values for this component's properties
UGoKartMovementReplicator::UGoKartMovementReplicator()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);
}


// Called when the game starts
void UGoKartMovementReplicator::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = GetOwner()->FindComponentByClass<UGoKartMovementCompoment>();
}


// Called every frame
void UGoKartMovementReplicator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MovementComponent)
	{
		FGoKartMove LastMove = MovementComponent->GetLastMove();

		// We are the server and in control of the pawn
		if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
		{
			UpdateServerState(LastMove);
		}

		if (GetOwnerRole() == ROLE_AutonomousProxy)
		{
			UnacknowledgedMoves.Add(LastMove);
			Server_SendMove(LastMove);
		}

		if (GetOwnerRole() == ROLE_SimulatedProxy)
		{
			ClientTick(DeltaTime);
		}
	}

	DrawDebugReplicationInfo();
}


void UGoKartMovementReplicator::ClientTick(float DeltaTime)
{
	ClientTimeSinceUpdate += DeltaTime;

	if (ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER) return;

	float LerpRatio = ClientTimeSinceUpdate / ClientTimeBetweenLastUpdates;

	FVector StartLocation = ClientStartTransform.GetLocation();
	FVector TargetLocation = ServerState.Transform.GetLocation();
	FVector NewLocation = FMath::LerpStable<FVector>(StartLocation, TargetLocation, LerpRatio);

	FQuat StartRotation = ClientStartTransform.GetRotation();
	FQuat TargetRotation = ServerState.Transform.GetRotation();
	FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);

	GetOwner()->SetActorLocationAndRotation(NewLocation, NewRotation);
}


// server only
void UGoKartMovementReplicator::UpdateServerState(const FGoKartMove& Move)
{
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}


// server only
void UGoKartMovementReplicator::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (MovementComponent)
	{
		MovementComponent->SimulateMove(Move);
		UpdateServerState(Move);
	}
}

bool UGoKartMovementReplicator::Server_SendMove_Validate(FGoKartMove Move)
{
	return true; // TODO  make better validation
}


// client only (notify method)
void UGoKartMovementReplicator::OnRep_ServerState()
{
	switch (GetOwnerRole())
	{
	case ROLE_AutonomousProxy:
		AutonomousProxy_OnRep_ServerState();
		break;

	case ROLE_SimulatedProxy:
		SimulatedProxy_OnRep_ServerState();
		break;
	}
}

void UGoKartMovementReplicator::AutonomousProxy_OnRep_ServerState()
{
	if (MovementComponent)
	{
		GetOwner()->SetActorTransform(ServerState.Transform);
		MovementComponent->SetVelocity(ServerState.Velocity);

		ClearAcknowledgedMoves(ServerState.LastMove);

		for (const FGoKartMove& Move : UnacknowledgedMoves)
		{
			MovementComponent->SimulateMove(Move);
		}
	}
}

void UGoKartMovementReplicator::SimulatedProxy_OnRep_ServerState()
{
	ClientTimeBetweenLastUpdates = ClientTimeSinceUpdate;
	ClientTimeSinceUpdate = 0.0f;
	ClientStartTransform = GetOwner()->GetActorTransform();
}


// client only
void UGoKartMovementReplicator::ClearAcknowledgedMoves(const FGoKartMove& LastMove)
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


void UGoKartMovementReplicator::DrawDebugReplicationInfo() const
{
	DrawDebugString(GetWorld(), FVector::UpVector * -20.0f, InsTestSpace::GetEnumText(GetOwnerRole()), GetOwner(), FColor::White, 0);
	DrawDebugString(GetWorld(), FVector::ZeroVector, FString::Printf(TEXT("Remote Role = %s"), *InsTestSpace::GetEnumText(GetOwner()->GetRemoteRole())), GetOwner(), FColor::Red, 0);
}


void UGoKartMovementReplicator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGoKartMovementReplicator, ServerState);
}
