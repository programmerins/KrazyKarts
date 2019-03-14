// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "GoKartMovementCompoment.h"

#include "GoKartMovementReplicator.generated.h"


// Receive to client (apply simulated proxy only)
USTRUCT()
struct FGoKartState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FGoKartMove LastMove;

	UPROPERTY()
	FVector_NetQuantize10 Velocity;

	UPROPERTY()
	FTransform Transform;
};


struct FHermiteCubicSpline
{
	FVector StartLocation;
	FVector	StartDerivative;
	FVector EndLocation;
	FVector EndDerivative;
	
	explicit FHermiteCubicSpline(FVector&& P0, FVector&& T0, FVector&& P1, FVector&& T1) :
		StartLocation(P0), StartDerivative(T0), EndLocation(P1), EndDerivative(T1)
	{ }

	FORCEINLINE FVector InterpolateLocation(float LerpRatio) const
	{
		return FMath::CubicInterp(StartLocation, StartDerivative, EndLocation, EndDerivative, LerpRatio);
	}

	FORCEINLINE FVector InterpolateDerivative(float LerpRatio) const
	{
		return FMath::CubicInterpDerivative(StartLocation, StartDerivative, EndLocation, EndDerivative, LerpRatio);
	}
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartMovementReplicator final : public UActorComponent
{
	GENERATED_BODY()


public:	
	// Sets default values for this component's properties
	UGoKartMovementReplicator();


protected:	
	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
		
	UFUNCTION(BlueprintCallable)
	void SetMeshOffsetRoot(USceneComponent* Root) { MeshOffsetRoot = Root; }


private:
	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FGoKartState ServerState;

	UPROPERTY()
	UGoKartMovementCompoment* MovementComponent;

	UPROPERTY()
	USceneComponent* MeshOffsetRoot;

	TArray<FGoKartMove> UnacknowledgedMoves;

	FTransform ClientStartTransform;

	FVector ClientStartVelocity;

	float ClientTimeSinceUpdate;

	float ClientTimeBetweenLastUpdates;

	float ClientSimulatedTime;

	
private:
	void ClientTick(float DeltaTime);

	void UpdateServerState(const FGoKartMove& Move);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove Move);

	UFUNCTION()
	void OnRep_ServerState();

	void AutonomousProxy_OnRep_ServerState();

	void SimulatedProxy_OnRep_ServerState();

	void ClearAcknowledgedMoves(const FGoKartMove& LastMove);

	void DrawDebugReplicationInfo() const;


	FORCEINLINE float GetVelocityToDerivative() const
	{
		return ClientTimeBetweenLastUpdates * 100.0f;
	}


	// wrapper func
	FORCEINLINE FHermiteCubicSpline CreateSpline()
	{
		return FHermiteCubicSpline
		(
			ClientStartTransform.GetLocation(),
			ClientStartVelocity * GetVelocityToDerivative(),
			ServerState.Transform.GetLocation(),
			ServerState.Velocity * GetVelocityToDerivative()
		);
	}


	// wrapper func
	FORCEINLINE void InterpolateLocation(const FHermiteCubicSpline& Spline, float LerpRatio) const
	{
		FVector NewLocation = Spline.InterpolateLocation(LerpRatio);

		if (MeshOffsetRoot)
		{
			MeshOffsetRoot->SetWorldLocation(NewLocation);
		}
	}


	// wrapper func
	FORCEINLINE void InterpolateRotation(float LerpRatio) const
	{
		FQuat StartRotation = ClientStartTransform.GetRotation();
		FQuat TargetRotation = ServerState.Transform.GetRotation();
		FQuat NewRotation = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);
 
		if (MeshOffsetRoot)
		{
			MeshOffsetRoot->SetWorldRotation(NewRotation);
		}
	}


	// wrapper func
	FORCEINLINE void InterpolateVelocity(const FHermiteCubicSpline& Spline, float LerpRatio) const
	{
		FVector NewDerivative = Spline.InterpolateDerivative(LerpRatio);
		FVector NewVelocity = NewDerivative / GetVelocityToDerivative();
		MovementComponent->SetVelocity(NewVelocity);
	}
};
