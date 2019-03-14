// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementCompoment.generated.h"


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


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class KRAZYKARTS_API UGoKartMovementCompoment : public UActorComponent
{
	GENERATED_BODY()


public:
	// Sets default values for this component's properties
	UGoKartMovementCompoment();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SimulateMove(const FGoKartMove& Move);

	FORCEINLINE FGoKartMove GetLastMove() const { return LastMove; }

	FORCEINLINE FVector GetVelocity() const { return Velocity; }

	FORCEINLINE void SetVelocity(FVector Value) { Velocity = Value; }

	FORCEINLINE void SetThrottle(float Value) { Throttle = Value; }
	
	FORCEINLINE void SetSteeringThrow(float Value) { SteeringThrow = Value; }


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

	FGoKartMove LastMove;

	FVector Velocity;

	float Throttle;

	float SteeringThrow;


private:
	FGoKartMove CreateMove(float DeltaTime) const;

	void UpdateLocationFromVelocity(float DeltaTime);

	void ApplyRotation(float DeltaTime, float SteeringThrow);


	FORCEINLINE FVector GetAirResistance() const
	{
		return -Velocity.GetSafeNormal() *  Velocity.SizeSquared() * DragCoefficient;
	};


	FORCEINLINE FVector GetRollingResistance() const
	{
		float AccelerationDueToGravity = -GetWorld()->GetGravityZ() * 0.01f;
		float NormalForce = Mass * AccelerationDueToGravity;

		return -Velocity.GetSafeNormal() * NormalForce * RollingResistanceCoefficient;
	}
};
