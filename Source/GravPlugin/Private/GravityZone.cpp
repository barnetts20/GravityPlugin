// Fill out your copyright notice in the Description page of Project Settings.


#include "GravityZone.h"

// Sets default values
AGravityZone::AGravityZone()
{
	PrimaryActorTick.bCanEverTick = true;
	Priority = 0;
	BaseVector = FVector(0, 0, -980);
}

// Called when the game starts or when spawned
void AGravityZone::BeginPlay()
{
	Super::BeginPlay();
	if (UWorld* World = GetWorld())
	{
		if (UGravityManager* GravityManager = World->GetSubsystem<UGravityManager>())
		{
			GravityManager->RegisterGravityZone(this);
		}
	}
}

void AGravityZone::BeginDestroy()
{
	if (UWorld* World = GetWorld())
	{
		if (UGravityManager* GravityManager = World->GetSubsystem<UGravityManager>())
		{
			GravityManager->UnregisterGravityZone(this);
		}
	}
	Super::BeginDestroy();
}


void AGravityZone::OnZoneBeginOverlap(AActor* OtherActor)
{
	if (UWorld* World = GetWorld())
	{
		if (UGravityManager* GravityManager = World->GetSubsystem<UGravityManager>())
		{
			GravityManager->NotifyObjectEnteredZone(OtherActor, this);
		}
	}
}

void AGravityZone::OnZoneEndOverlap(AActor* OtherActor)
{
	if (UWorld* World = GetWorld())
	{
		if (UGravityManager* GravityManager = World->GetSubsystem<UGravityManager>())
		{
			GravityManager->NotifyObjectLeftZone(OtherActor, this);
		}
	}
}

FVector AGravityZone::GetGravityVector_Implementation(const FVector& InWorldLocation) const
{
	return BaseVector;
}
