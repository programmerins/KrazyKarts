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


private:
	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FGoKartState ServerState;

	UPROPERTY()
	UGoKartMovementCompoment* MovementComponent;

	TArray<FGoKartMove> UnacknowledgedMoves;

	FTransform ClientStartTransform;

	float ClientTimeSinceUpdate;

	float ClientTimeBetweenLastUpdates;

	
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
};
