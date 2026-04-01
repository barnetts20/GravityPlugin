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
            //Transition towards the desired gravity vector
            GravityDirection = FMath::Lerp(LastFrameGravity, MoveComp->GetGravityDirection(), DeltaTime * DeltaSmoothing);
        }
    }

    // Get the current control rotation in world space
    FRotator ViewRotation = GetControlRotation();
    Pitch += RotationInput.Pitch;
    Pitch = FMath::Clamp(Pitch, -MaxPitch, MaxPitch);
    if (!LastFrameGravity.Equals(GravityDirection))
    {
        const FQuat CurrentRotationQuat = FQuat::FindBetweenNormals(LastFrameGravity, GravityDirection);
        FQuat TargetRotationQuat = CurrentRotationQuat * FQuat(ViewRotation);

        // Interpolate using Slerp for smooth transition
        FQuat SmoothedRotation = FQuat::Slerp(FQuat(ViewRotation), TargetRotationQuat, DeltaTime);
        ViewRotation = SmoothedRotation.Rotator();
    }

    LastFrameGravity = GravityDirection;

    // Convert the view rotation from world space to gravity relative space
    ViewRotation = GetGravityRelativeRotation(ViewRotation, GravityDirection);

    FRotator DeltaRot(RotationInput);

    if (PlayerCameraManager)
    {
        ACharacter* PlayerCharacter = Cast<ACharacter>(GetPawn());

        // Keep the camera roll level with gravity
        ViewRotation.Roll = 0;
        ViewRotation.Pitch = Pitch;

        // Convert back to world space
        PlayerCameraManager->ProcessViewRotation(DeltaTime, ViewRotation, DeltaRot);
        SetControlRotation(GetGravityWorldRotation(ViewRotation, GravityDirection));
    }

    APawn* const P = GetPawnOrSpectator();
    if (P)
    {
        P->FaceRotation(ViewRotation, DeltaTime);
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