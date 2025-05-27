// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GravityManager.h"
#include "GravityZone.generated.h"

UCLASS()
class GRAVPLUGIN_API AGravityZone : public AActor
{
	GENERATED_BODY()
	
public:	
	AGravityZone();

protected:
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

	//Overlap Bindings
	UFUNCTION(BlueprintCallable, Category = "Gravity Zone")
	void OnZoneBeginOverlap(AActor* OtherActor);

	UFUNCTION(BlueprintCallable, Category = "Gravity Zone")
	void OnZoneEndOverlap(AActor* OtherActor);

public:
	//The zone priority, only the highest level zones an actor occupies will apply force
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Zone")
	int Priority;

	//Defines a base vector, depending on implementation of GetGravityVector, this could be used directly, could be used to derive a magnitude, direction, etc
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Zone")
	FVector BaseVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Zone")
	float LinearDamping; // Linear damping applied to physics objects in this zone

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Zone")
	float AngularDamping; // Angular damping applied to physics objects in this zone

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gravity Zone")
	TArray<FString> ExclusionTags; // Angular damping applied to physics objects in this zone

	//Override to change the way the gravity is calculated given an obect at a world position
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Gravity Zone")
	FVector GetGravityVector(const FVector& InWorldPosition) const;

	//Override to change the way the gravity is calculated given an obect at a world position
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Gravity Zone")
	double GetLinearDampening(const FVector& InWorldPosition) const;

	//Override to change the way the gravity is calculated given an obect at a world position
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Gravity Zone")
	double GetAngularDampening(const FVector& InWorldPosition) const;
};
