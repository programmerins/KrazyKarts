// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "GoKartMovementReplicator.h"
#include "GoKartMovementCompoment.h"

#include "GoKart.generated.h"


// fwd
class UGoKartMovementReplicator;


UCLASS()
class KRAZYKARTS_API AGoKart final : public APawn
{
	GENERATED_BODY()


public:
	// Sets default values for this pawn's properties
	AGoKart();


public:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


protected:
/*  Dependency Components Association */
/// this -> MovementComponent
/// this -> MovementReplicator
/// MovementReplicator -> MovementCompoment
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UGoKartMovementCompoment* MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UGoKartMovementReplicator* MovementReplicator;


private:
	void MoveForward(float Axis);

	void MoveRight(float Axis);
};
