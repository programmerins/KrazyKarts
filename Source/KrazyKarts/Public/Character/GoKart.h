// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

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

	float Throttle;

	float SteeringThrow;

	FVector Velocity;


private:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveForward(float Axis);

	void MoveRight(float Axis);

	void UpdateLocationFromVelocity(const FVector &Translation);

	void ApplyRotation(float DeltaTime);

	FVector GetAirResistance();

	FVector GetRollingResistance();
};
