// --- GravityManager.cpp ---
#include "GravityManager.h"
#include "GravityZone.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

// --- Subsystem Lifecycle ---
void UGravityManager::Initialize(FSubsystemCollectionBase& Collection) // Make sure this matches your class name
{
	Super::Initialize(Collection);
	if (UWorld* World = GetWorld())
	{
		for (AGravityZone* Zone : RegisteredGravityZones)
		{
			if (Zone)
			{
				TArray<AActor*> OverlappingActors;
				Zone->GetOverlappingActors(OverlappingActors);
				for (AActor* OverlappingActor : OverlappingActors)
				{
					NotifyObjectEnteredZone(OverlappingActor, Zone);
				}
			}
		}
	}
	UE_LOG(LogTemp, Log, TEXT("GravityManager Initialized")); // Use your class name in logs
}

void UGravityManager::Deinitialize() // Make sure this matches your class name
{
	UE_LOG(LogTemp, Log, TEXT("GravityManager Deinitialized")); // Use your class name in logs
	RegisteredGravityZones.Empty();
	ActorZoneOverlaps.Empty();
	Super::Deinitialize();
}

bool UGravityManager::ShouldCreateSubsystem(UObject* Outer) const // Make sure this matches your class name
{
	if (UWorld* World = Cast<UWorld>(Outer))
	{
		return World->IsGameWorld();
	}
	return false;
}

// --- Gravity Zone Management ---
void UGravityManager::RegisterGravityZone(AGravityZone* GravityZone) // Make sure this matches your class name
{
	if (GravityZone)
	{
		RegisteredGravityZones.Add(GravityZone);
		TArray<AActor*> OverlappingActors;
		GravityZone->GetOverlappingActors(OverlappingActors);
		for (AActor* OverlappingActor : OverlappingActors)
		{
			NotifyObjectEnteredZone(OverlappingActor, GravityZone);
		}
		UE_LOG(LogTemp, Log, TEXT("Registered Gravity Zone: %s"), *GravityZone->GetName());
	}
}
void UGravityManager::UnregisterGravityZone(AGravityZone* GravityZone) // Make sure this matches your class name
{
	if (GravityZone)
	{
		RegisteredGravityZones.Remove(GravityZone);
		for (auto& Pair : ActorZoneOverlaps)
		{
			Pair.Value.Remove(GravityZone);
		}
		ActorZoneOverlaps.Remove(nullptr);
		for (auto It = ActorZoneOverlaps.CreateIterator(); It; ++It)
		{
			if (It.Value().Num() == 0)
			{
				It.RemoveCurrent();
			}
		}
		UE_LOG(LogTemp, Log, TEXT("Unregistered Gravity Zone: %s"), *GravityZone->GetName());
	}
}

// --- Object Overlap Notification ---
void UGravityManager::NotifyObjectEnteredZone(AActor* AffectedActor, AGravityZone* GravityZone)
{
	if (AffectedActor && GravityZone && (Cast<ACharacter>(AffectedActor) || AffectedActor->FindComponentByClass<UPrimitiveComponent>()))
	{
		// Check if any tag of the AffectedActor is in the ExcludeTags list
		for (const auto Tag : AffectedActor->Tags)
		{
			if (GravityZone->ExcludeTags.Contains(Tag)) return;
		}

		ActorZoneOverlaps.FindOrAdd(AffectedActor).Add(GravityZone);
	}
}

void UGravityManager::NotifyObjectLeftZone(AActor* AffectedActor, AGravityZone* GravityZone)
{
	if (AffectedActor && GravityZone)
	{
		if (TSet<AGravityZone*>* OverlappingZones = ActorZoneOverlaps.Find(AffectedActor))
		{
			OverlappingZones->Remove(GravityZone);
		}
	}
}

// --- Tick Function ---
void UGravityManager::Tick(float DeltaTime)
{
	TArray<AActor*> ActorsToProcess;
	ActorZoneOverlaps.GetKeys(ActorsToProcess);

	for (AActor* AffectedActor : ActorsToProcess)
	{
		if (!AffectedActor)
		{
			ActorZoneOverlaps.Remove(AffectedActor);
			continue;
		}
		FVector NetGravityVector = CalculateNetGravityVectorForActor(AffectedActor);
		ApplyGravityToActorComponents(AffectedActor, NetGravityVector);
	}
}

TStatId UGravityManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UGravityManager, STATGROUP_Tickables);
}

// --- Gravity Application Logic ---
FVector UGravityManager::CalculateNetGravityVectorForActor(AActor* AffectedActor) const
{
	FVector NetGravity = FVector::ZeroVector;
	FVector MaxGravity = FVector::ZeroVector;
	//Early termination conditions
	if (!AffectedActor) return NetGravity;
	const TSet<AGravityZone*>* OverlappingZones = ActorZoneOverlaps.Find(AffectedActor);
	if (!OverlappingZones || OverlappingZones->Num() == 0)
	{
		return NetGravity;
	}

	// Check if the actor is a Character and if they are "grounded"
	bool bIsCharacterGrounded = false;
	ACharacter* Character = Cast<ACharacter>(AffectedActor);
	if (Character)
	{
		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			bIsCharacterGrounded = !MovementComp->IsFalling();
		}
	}

	//Accumulate gravity in one pass
	FVector ActorLocation = AffectedActor->GetActorLocation();
	int32 HighestPriority = -INT_MAX;
	for (AGravityZone* Zone : *OverlappingZones)
	{
		if (Zone) {
			FVector ZoneGravity = Zone->GetGravityVector(ActorLocation);
			if (Zone->Priority > HighestPriority)
			{
				NetGravity = FVector::ZeroVector;
				MaxGravity = FVector::ZeroVector;
				HighestPriority = Zone->Priority;
				NetGravity = ZoneGravity;
				MaxGravity = ZoneGravity;
			}
			else if (Zone->Priority == HighestPriority) {
				MaxGravity = (MaxGravity.Size() < ZoneGravity.Size() ? ZoneGravity : MaxGravity);
				//If it is a character and the character is "grounded" we only apply the strongest gravity vector
				//This stops the character from "leaning" towards the shared center of gravity when walking on an object
				if (bIsCharacterGrounded) {
					NetGravity = MaxGravity;
				}
				else {
					NetGravity += ZoneGravity;
				}
			}
		}	
	}
	return NetGravity;
}

void UGravityManager::ApplyGravityToActorComponents(AActor* AffectedActor, const FVector& NetGravityVector)
{
	if (!AffectedActor) return;

	// --- Case 1: Third Person Gravity Character ---
	if (ACharacter* Character = Cast<ACharacter>(AffectedActor))
	{
		USkeletalMeshComponent* SkMesh = Character->GetMesh();
		if (SkMesh && SkMesh->IsSimulatingPhysics()) {
			for (auto Body : SkMesh->Bodies) {
				Body->AddForce(Body->GetBodyMass() * Body->MassScale * NetGravityVector);
			}
		} else if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			MovementComp->GravityScale = NetGravityVector.Size() / 980.0; // Assuming 1.0 is the default scale, 9.8 m/s as our baseline
			auto gravDir = NetGravityVector.GetSafeNormal();
			MovementComp->SetGravityDirection(gravDir.IsNearlyZero() ? Character->GetActorUpVector() * -1.0 : gravDir);
		}
		return;
	}

	// --- Case 2: Primitive Components ---
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	AffectedActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
	for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
	{
		if (PrimComp && PrimComp->IsSimulatingPhysics() && !PrimComp->IsGravityEnabled())
		{
			if (float Mass = PrimComp->GetMass() > KINDA_SMALL_NUMBER && !NetGravityVector.IsNearlyZero())
			{
				FVector GravityForce = NetGravityVector * Mass * PrimComp->GetMassScale();
				PrimComp->AddImpulse(GravityForce, NAME_None, false);
			}
		}
	}
}

// --- Blueprint Access ---
UGravityManager* UGravityManager::GetGravityManagerSubsystem(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
	{
		return World->GetSubsystem<UGravityManager>();
	}
	return nullptr;
}
