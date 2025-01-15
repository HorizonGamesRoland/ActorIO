// Copyright 2025 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicSpawner.generated.h"

/** Delegate when an actor is spawned. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLogicSpawnerSpawnActor, AActor*, ActorPtr);

/**
 * An actor entry for the logic spawner.
 */
USTRUCT(BlueprintType)
struct FLogicSpawnerEntry
{
    GENERATED_BODY()

    /** Actor class to spawn. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    TSubclassOf<AActor> SpawnClass;

    /** Spawn point of the actor. Use the built in Target Point actor in Unreal to easily mark spawn points. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    TObjectPtr<AActor> SpawnPoint;

    /** Default constructor. */
    FLogicSpawnerEntry() :
        SpawnClass(TSubclassOf<AActor>()),
        SpawnPoint(nullptr)
    {}

    /** Constructor from a class and spawn point. */
    FLogicSpawnerEntry(UClass* InSpawnClass, AActor* InSpawnPoint) :
        SpawnClass(InSpawnClass),
        SpawnPoint(InSpawnPoint)
    {}
};

/**
 * An actor that can spawn other actors at runtime.
 */
UCLASS()
class ACTORIO_API ALogicSpawner : public ALogicActorBase
{
    GENERATED_BODY()

public:

    /** Default constructor. */
    ALogicSpawner();

public:

    /** List of actors to spawn. */
    UPROPERTY(EditInstanceOnly, Category = "Spawner")
    TArray<FLogicSpawnerEntry> ActorsToSpawn;

    /** Whether to spawn the actors automatically during 'BeginPlay'. */
    UPROPERTY(EditInstanceOnly, Category = "Spawner")
    bool bSpawnActorsOnStart;

    /** Event whenever a new actor is spawned */
    UPROPERTY(BlueprintAssignable, Category = "Spawner")
    FOnLogicSpawnerSpawnActor OnActorSpawned;

    /** Event when all actors have finished spawning. */
    UPROPERTY(BlueprintAssignable, Category = "Spawner")
    FSimpleActionDelegate OnSpawnFinished;

protected:

    /** Reference to all actors spawned by the spawner. */
    UPROPERTY(Transient)
    TArray<TObjectPtr<AActor>> SpawnedActors;

public:

    //~ Begin ALogicActorBase Interface
    virtual void BeginPlay() override;
    virtual void RegisterIOEvents_Implementation(FActorIOEventList& RegisteredEvents) override;
    virtual void RegisterIOFunctions_Implementation(FActorIOFunctionList& RegisteredFunctions) override;
    //~ End ALogicActorBase Interface

public:

    /** Spawn the actors. */
    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void Spawn();

    /** Get a reference to the actor that has been spawned for the given entry. */
    UFUNCTION(BlueprintPure, Category = "Spawner")
    AActor* GetSpawnedActorAt(int32 Index) const;

    /** Get the number of spawned actors. */
    UFUNCTION(BlueprintPure, Category = "Spawner")
    int32 GetSpawnedActorCount() const;

protected:

    /** Event processor for the 'OnActorSpawned' event. */
    UFUNCTION()
    void ProcessEvent_OnActorSpawned(AActor* ActorPtr);
};