// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"


// Receive to server
USTRUCT()
struct FGoKartMove
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float SteeringThrow;

	UPROPERTY()
	float DeltaTime;

	UPROPERTY()
	float Time;
};


// Receive to client (only apply simulated proxy)
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


UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()


public:
	// Sets default values for this pawn's properties
	AGoKart();


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


private:
	/* The mass of the car (kg) */
	UPROPERTY(EditDefaultsOnly)
	float Mass = 1000.0f;

	/* The force applied to the car when the throttle is fully down (N) */
	UPROPERTY(EditDefaultsOnly)
	float MaxDrivingForce = 10000.0f;

	/* Minimum radius of the car turning circle at full lock (m) */
	UPROPERTY(EditDefaultsOnly)
	float MinTurningRadius = 10.0f;

	/* Higher means more drag */
	/* DragCoefficient is indeed unitless! 0.3f - 0.5f is generally good to use. 0.3f - Tanks, 0.5f - Automobiles */
	UPROPERTY(EditDefaultsOnly)
	float DragCoefficient = 16.0f;

	/* Higher means more rolling resistance */
	/* Ordinary car tires on concrete */
	float RollingResistanceCoefficient = 0.015f;

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FGoKartState ServerState;

	FVector Velocity;

	float Throttle;

	float SteeringThrow;

	TArray<FGoKartMove> UnacknowledgedMoves;


private:
	UFUNCTION()
	void OnRep_ServerState();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove Move);

	FGoKartMove CreateMove(float DeltaTime) const;

	void ClearAcknowledgedMoves(const FGoKartMove& LastMove);

	void SimulateMove(FGoKartMove  Move);

	void MoveForward(float Axis);

	void MoveRight(float Axis);

	void UpdateLocationFromVelocity(float DeltaTime);

	void ApplyRotation(float DeltaTime, float SteeringThrow);

	FVector GetAirResistance();

	FVector GetRollingResistance();
};
