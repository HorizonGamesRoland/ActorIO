// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "LogicActors/LogicSpawner.h"
#include "Engine/World.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicSpawner::ALogicSpawner()
{
	ActorsToSpawn = TArray<FLogicSpawnerEntry>();
	bSpawnActorsOnStart = true;

#if WITH_EDITORONLY_DATA
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/ActorIO/AssetIcons/S_Spawner"));
	if (SpriteComponent && SpriteTexture.Succeeded())
	{
		SpriteComponent->SetSprite(SpriteTexture.Get());
		SpriteComponent->SetRelativeScale3D_Direct(FVector(1.0f));
	}
#endif
}

void ALogicSpawner::RegisterIOEvents(FActorIOEventList& EventRegistry)
{
	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicSpawner::OnActorSpawned"))
		.SetDisplayName(LOCTEXT("ALogicSpawner.OnActorSpawned", "OnActorSpawned"))
		.SetTooltipText(LOCTEXT("ALogicSpawner.OnActorSpawnedTooltip", "Event whenever a new actor is spawned."))
		.SetMulticastDelegate(this, &OnActorSpawned)
		.SetEventProcessor(this, TEXT("ProcessEvent_OnActorSpawned")));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicSpawner::OnSpawnFinished"))
		.SetDisplayName(LOCTEXT("ALogicSpawner.OnSpawnFinished", "OnSpawnFinished"))
		.SetTooltipText(LOCTEXT("ALogicSpawner.OnSpawnFinishedTooltip", "Event when all actors have finished spawning."))
		.SetMulticastDelegate(this, &OnSpawnFinished));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicSpawner::OnGetSpawnedActor"))
		.SetDisplayName(LOCTEXT("ALogicSpawner.OnGetSpawnedActor", "OnGetSpawnedActor"))
		.SetTooltipText(LOCTEXT("ALogicSpawner.OnGetSpawnedActorTooltip", "Event when getting a spawned actor using the 'GetSpawnedActorForEntry' function."))
		.SetMulticastDelegate(this, &OnGetSpawnedActor)
		.SetEventProcessor(this, TEXT("ProcessEvent_OnGetSpawnedActor")));
}

void ALogicSpawner::RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry)
{
	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicSpawner::SpawnActors"))
		.SetDisplayName(LOCTEXT("ALogicSpawner.SpawnActors", "SpawnActors"))
		.SetTooltipText(LOCTEXT("ALogicSpawner.SpawnTooltip", "Spawn the actors."))
		.SetFunction(TEXT("SpawnActors")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicSpawner::DestroySpawnedActors"))
		.SetDisplayName(LOCTEXT("ALogicSpawner.DestroySpawnedActors", "DestroySpawnedActors"))
		.SetTooltipText(LOCTEXT("ALogicSpawner.DestroySpawnedActorsTooltip", "Destroy all spawned actors."))
		.SetFunction(TEXT("DestroySpawnedActors")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicSpawner::DestroySpawnedActorForEntry"))
		.SetDisplayName(LOCTEXT("ALogicSpawner.DestroySpawnedActorForEntry", "DestroySpawnedActorForEntry"))
		.SetTooltipText(LOCTEXT("ALogicSpawner.DestroySpawnedActorForEntryTooltip", "Destroy the actor that was spawned for the given entry."))
		.SetFunction(TEXT("DestroySpawnedActorForEntry")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicSpawner::GetSpawnedActorForEntry"))
		.SetDisplayName(LOCTEXT("ALogicSpawner.GetSpawnedActorForEntry", "GetSpawnedActorForEntry"))
		.SetTooltipText(LOCTEXT("ALogicSpawner.GetSpawnedActorForEntryTooltip", "Get the actor that was spawned for the given entry and fire 'OnGetSpawnedActor' event."))
		.SetFunction(TEXT("GetSpawnedActorForEntry")));
}

void ALogicSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnActorsOnStart)
	{
		SpawnActors();
	}
}

void ALogicSpawner::SpawnActors()
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	for (int32 EntryIdx = 0; EntryIdx != ActorsToSpawn.Num(); ++EntryIdx)
	{
		const FLogicSpawnerEntry& SpawnEntry = ActorsToSpawn[EntryIdx];
		if (SpawnEntry.IsValid())
		{
			if (SpawnEntry.SpawnDelay > 0.0f)
			{
				FTimerHandle UniqueHandle;
				FTimerDelegate SpawnDelegate = FTimerDelegate::CreateUObject(this, &ThisClass::InternalSpawnActor, EntryIdx);
				GetWorld()->GetTimerManager().SetTimer(UniqueHandle, SpawnDelegate, SpawnEntry.SpawnDelay, false);
			}
			else
			{
				InternalSpawnActor(EntryIdx);
			}
		}
	}

	OnSpawnFinished.Broadcast();
}

void ALogicSpawner::InternalSpawnActor(int32 EntryIdx)
{
	FActorSpawnParameters SpawnParams = FActorSpawnParameters();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

	const FLogicSpawnerEntry& SpawnEntry = ActorsToSpawn[EntryIdx];
	AActor* NewActor = GetWorld()->SpawnActor<AActor>(
		SpawnEntry.SpawnClass,
		SpawnEntry.SpawnPoint->GetActorLocation(),
		SpawnEntry.SpawnPoint->GetActorRotation(),
		SpawnParams);

	SpawnedActors.Add(NewActor);
	OnActorSpawned.Broadcast(NewActor);
}

void ALogicSpawner::DestroySpawnedActors()
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	for (AActor* SpawnedActor : SpawnedActors)
	{
		if (IsValid(SpawnedActor))
		{
			GetWorld()->DestroyActor(SpawnedActor);
		}
	}
}

void ALogicSpawner::DestroySpawnedActorForEntry(int32 EntryIdx)
{
	AActor* ActorToDestroy = GetSpawnedActorForEntry(EntryIdx);
	if (IsValid(ActorToDestroy))
	{
		GetWorld()->DestroyActor(ActorToDestroy);
	}
}

AActor* ALogicSpawner::GetSpawnedActorForEntry(int32 EntryIdx) const
{
	AActor* OutActor = nullptr;
	if (SpawnedActors.IsValidIndex(EntryIdx))
	{
		OutActor = SpawnedActors[EntryIdx];
	}

	OnGetSpawnedActor.Broadcast(OutActor);
	return OutActor;
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

void ALogicSpawner::ProcessEvent_OnGetSpawnedActor(AActor* ActorPtr)
{
	FActionExecutionContext& ExecContext = FActionExecutionContext::Get(this);
	ExecContext.SetNamedArgument(TEXT("$Actor"), IsValid(ActorPtr) ? ActorPtr->GetPathName() : FString());
}

#undef LOCTEXT_NAMESPACE
