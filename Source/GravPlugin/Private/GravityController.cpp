#include "GravityController.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void AGravityController::UpdateRotation(float DeltaTime)
{
    FVector GravityDirection = FVector::DownVector;
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(GetPawn()))
    {
        if (UCharacterMovementComponent* MoveComp = PlayerCharacter->GetCharacterMovement())
        {
            FVector TargetGravity = MoveComp->GetGravityDirection();
            // Smoothly transition toward the current gravity direction.
            // Normalize after lerp to prevent drift off unit length.
            GravityDirection = FMath::Lerp(LastFrameGravity, TargetGravity, FMath::Clamp(DeltaTime * DeltaSmoothing, 0.0, 1.0));
            if (!GravityDirection.Normalize())
            {
                GravityDirection = TargetGravity;
            }
        }
    }

    // Get the current control rotation and convert to gravity-relative space
    FRotator ViewRotation = GetControlRotation();

    // If gravity direction changed, rotate the view to follow the gravity shift.
    // Work in quaternion space for stability.
    if (!LastFrameGravity.Equals(GravityDirection, 0.0001))
    {
        FQuat GravityShift = FQuat::FindBetweenNormals(LastFrameGravity, GravityDirection);
        ViewRotation = (GravityShift * ViewRotation.Quaternion()).Rotator();
    }

    LastFrameGravity = GravityDirection;

    // Convert to gravity-relative space for pitch/roll manipulation
    FRotator GravRelativeView = GetGravityRelativeRotation(ViewRotation, GravityDirection);

    // Accumulate pitch input and apply yaw from RotationInput
    Pitch += RotationInput.Pitch;
    Pitch = FMath::Clamp(Pitch, -MaxPitch, MaxPitch);

    // Force pitch from our accumulator, zero roll to stay level with gravity,
    // and apply yaw input.
    GravRelativeView.Pitch = Pitch;
    GravRelativeView.Roll = 0.0;
    GravRelativeView.Yaw += RotationInput.Yaw;

    // Convert back to world space
    FRotator WorldViewRotation = GetGravityWorldRotation(GravRelativeView, GravityDirection);

    if (PlayerCameraManager)
    {
        SetControlRotation(WorldViewRotation);
    }

    APawn* const P = GetPawnOrSpectator();
    if (P)
    {
        P->FaceRotation(WorldViewRotation, DeltaTime);
    }
}

//Convert from world rotation to gravity relative rotation
FRotator AGravityController::GetGravityRelativeRotation(FRotator Rotation, FVector GravityDirection)
{
    if (!GravityDirection.Equals(FVector::DownVector))
    {
        FQuat GravityRotation = FQuat::FindBetweenNormals(GravityDirection, FVector::DownVector);
        return (GravityRotation * Rotation.Quaternion()).Rotator();
    }

    return Rotation;
}

//Convert from gravity relative rotation to world rotation
FRotator AGravityController::GetGravityWorldRotation(FRotator Rotation, FVector GravityDirection)
{
    if (!GravityDirection.Equals(FVector::DownVector))
    {
        FQuat GravityRotation = FQuat::FindBetweenNormals(FVector::DownVector, GravityDirection);
        return (GravityRotation * Rotation.Quaternion()).Rotator();
    }

    return Rotation;
}

FRotator AGravityController::GetDesiredMovementRotation() const
{
    // Get the current gravity direction
    FVector GravityDirection = FVector::DownVector;
    if (ACharacter* PlayerCharacter = Cast<ACharacter>(GetPawn()))
    {
        if (UCharacterMovementComponent* MoveComp = PlayerCharacter->GetCharacterMovement())
        {
            GravityDirection = MoveComp->GetGravityDirection();
        }
    }

    // Convert control rotation to gravity-relative space, zero out pitch,
    // then convert back. This gives us the yaw-only rotation on the gravity
    // plane — movement direction is independent of where the camera looks vertically.
    FRotator GravRelative = GetGravityRelativeRotation(GetControlRotation(), GravityDirection);
    GravRelative.Pitch = 0.0;
    GravRelative.Roll = 0.0;
    return GetGravityWorldRotation(GravRelative, GravityDirection);
}