// --- GravityManager.h ---

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GravityManager.generated.h"

class AGravityZone;
class UPrimitiveComponent;
class AActor;

/**
 * UWorldSubsystem that manages custom gravity zones and applies gravity to affected objects.
 * Functions are exposed to Blueprint for easier interaction.
 */
UCLASS(Blueprintable)
class UGravityManager : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	// --- Subsystem Lifecycle ---
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Gravity Zone Management ---
	UFUNCTION(BlueprintCallable, Category = "Gravity Manager")
	void RegisterGravityZone(AGravityZone* GravityZone);

	UFUNCTION(BlueprintCallable, Category = "Gravity Manager")
	void UnregisterGravityZone(AGravityZone* GravityZone);

	// --- Object Overlap Notification ---
	UFUNCTION(BlueprintCallable, Category = "Gravity Manager")
	void NotifyObjectEnteredZone(AActor* AffectedActor, AGravityZone* GravityZone);

	UFUNCTION(BlueprintCallable, Category = "Gravity Manager")
	void NotifyObjectLeftZone(AActor* AffectedActor, AGravityZone* GravityZone);

	// --- Tick Function ---
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// --- Blueprint Access ---
	UFUNCTION(BlueprintPure, Category = "Gravity Manager", meta = (WorldContext = "WorldContextObject"))
	static UGravityManager* GetGravityManagerSubsystem(const UObject* WorldContextObject);


private:
	// --- Internal Data Structures ---
	TSet<AGravityZone*> RegisteredGravityZones;
	TMap<AActor*, TSet<AGravityZone*>> ActorZoneOverlaps;

	// --- Gravity Application Logic ---
	FVector CalculateNetGravityVectorForActor(AActor* AffectedActor) const;
	void ApplyGravityToActorComponents(AActor* AffectedActor, const FVector& NetGravityVector);
};
