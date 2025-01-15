// Copyright 2025 Horizon Games. All Rights Reserved.

#include "LogicActors/LogicSpawner.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicSpawner::ALogicSpawner()
{
	ActorsToSpawn = TArray<FLogicSpawnerEntry>();
	bSpawnOnStart = true;
}

void ALogicSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnOnStart)
	{
		Spawn();
	}
}

void ALogicSpawner::RegisterIOEvents_Implementation(FActorIOEventList& RegisteredEvents)
{
	RegisteredEvents.Add(FActorIOEvent()
		.SetId(TEXT("ALogicSpawner::OnActorSpawned"))
		.SetDisplayName(LOCTEXT("ALogicSpawner.OnActorSpawned", "OnActorSpawned"))
		.SetTooltipText(LOCTEXT("ALogicSpawner.OnActorSpawnedTooltip", "Event whenever a new actor is spawned."))
		.SetMulticastDelegate(this, &OnActorSpawned)
		.SetEventProcessor(this, TEXT("ProcessEvent_OnActorSpawned")));

	RegisteredEvents.Add(FActorIOEvent()
		.SetId(TEXT("ALogicSpawner::OnSpawnFinished"))
		.SetDisplayName(LOCTEXT("ALogicSpawner.OnSpawnFinished", "OnSpawnFinished"))
		.SetTooltipText(LOCTEXT("ALogicSpawner.OnSpawnFinishedTooltip", "Event when all actors have finished spawning."))
		.SetMulticastDelegate(this, &OnSpawnFinished));
}

void ALogicSpawner::RegisterIOFunctions_Implementation(FActorIOFunctionList& RegisteredFunctions)
{
	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicSpawner::Spawn"))
		.SetDisplayName(LOCTEXT("ALogicSpawner.Spawn", "Spawn"))
		.SetTooltipText(LOCTEXT("ALogicSpawner.SpawnTooltip", "Spawn the actors."))
		.SetFunction(TEXT("Spawn")));
}

void ALogicSpawner::Spawn()
{
	FActorSpawnParameters SpawnParams = FActorSpawnParameters();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	UWorld* MyWorld = GetWorld();
	for (const FLogicSpawnerEntry& SpawnEntry : ActorsToSpawn)
	{
		if (SpawnEntry.SpawnClass && IsValid(SpawnEntry.SpawnPoint))
		{
			AActor* NewActor = MyWorld->SpawnActor<AActor>(
				SpawnEntry.SpawnClass,
				SpawnEntry.SpawnPoint->GetActorLocation(),
				SpawnEntry.SpawnPoint->GetActorRotation(),
				SpawnParams);

			SpawnedActors.Add(NewActor);
			OnActorSpawned.Broadcast(NewActor);
		}
	}

	OnSpawnFinished.Broadcast();
}

AActor* ALogicSpawner::GetSpawnedActorAt(int32 Index) const
{
	if (SpawnedActors.IsValidIndex(Index))
	{
		return SpawnedActors[Index];
	}

	return nullptr;
}

int32 ALogicSpawner::GetSpawnedActorCount() const
{
	return SpawnedActors.Num();
}

void ALogicSpawner::ProcessEvent_OnActorSpawned(AActor* ActorPtr)
{
	FActionExecutionContext& ExecContext = FActionExecutionContext::Get(this);
	ExecContext.SetNamedArgument(TEXT("$Actor"), IsValid(ActorPtr) ? ActorPtr->GetPathName() : FString());
}

#undef LOCTEXT_NAMESPACE
