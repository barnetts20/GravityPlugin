#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GravityController.generated.h"

/**
 * A Player Controller class which adds input-handling functionality for
 * CharacterMovementController's custom gravity mechanics.
 */
UCLASS()
class AGravityController : public APlayerController
{
	GENERATED_BODY()

public:
	double Pitch = 0; //Track input pitch accumulation independantly from controller rotation
	double MaxPitch = 70; //Max camera pitch, at >= 90 gimbal locking can occur
	double DeltaSmoothing = .1; //Estimated duration of transition between gravity vectors in seconds

	FVector TransitionGravityDirection = FVector::DownVector;

	virtual void UpdateRotation(float DeltaTime) override;

	// Converts a rotation from world space to gravity relative space.
	UFUNCTION(BlueprintPure)
	static FRotator GetGravityRelativeRotation(FRotator Rotation, FVector GravityDirection);

	// Converts a rotation from gravity relative space to world space.
	UFUNCTION(BlueprintPure)
	static FRotator GetGravityWorldRotation(FRotator Rotation, FVector GravityDirection);

private:
	FVector LastFrameGravity = FVector::ZeroVector;
};