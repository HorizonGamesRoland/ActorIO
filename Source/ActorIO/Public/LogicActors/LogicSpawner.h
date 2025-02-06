// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicSpawner.generated.h"

/** Delegate when an actor is spawned. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpawnerSpawnActor, AActor*, ActorPtr);
/** Delegate when getting a spawned actor. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGetSpawnedActor, AActor*, ActorPtr);

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

    /**
     * Spawn point of the actor.
     * Use the built in Target Point actor in Unreal to easily mark spawn points.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    TObjectPtr<AActor> SpawnPoint;

    /** Time delay before spawning the actor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    float SpawnDelay;

    /** Default constructor. */
    FLogicSpawnerEntry() :
        SpawnClass(TSubclassOf<AActor>()),
        SpawnPoint(nullptr),
        SpawnDelay(0.0f)
    {}

    /** Constructor from already existing params. */
    FLogicSpawnerEntry(UClass* InSpawnClass, AActor* InSpawnPoint, float InSpawnDelay) :
        SpawnClass(InSpawnClass),
        SpawnPoint(InSpawnPoint),
        SpawnDelay(InSpawnDelay)
    {}

    /** @return Whether an actor is spawnable from the entry. */
    FORCEINLINE bool IsValid() const
    {
        return SpawnClass != nullptr && SpawnPoint != nullptr;
    }
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
    FOnSpawnerSpawnActor OnActorSpawned;

    /** Event when all actors have finished spawning. */
    UPROPERTY(BlueprintAssignable, Category = "Spawner")
    FSimpleActionDelegate OnSpawnFinished;

    /** Event when getting a spawned actor using the 'GetSpawnedActorForEntry' function. */
    UPROPERTY(BlueprintAssignable, Category = "Spawner")
    FOnGetSpawnedActor OnGetSpawnedActor;

protected:

    /** Reference to all actors spawned by the spawner. */
    UPROPERTY(Transient)
    TArray<TObjectPtr<AActor>> SpawnedActors;

protected:

    //~ Begin ALogicActorBase Interface
    virtual void RegisterIOEvents(FActorIOEventList& EventRegistry) override;
    virtual void RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry) override;
    virtual void BeginPlay() override;
    //~ End ALogicActorBase Interface

public:

    /** Spawn the actors. */
    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void SpawnActors();

    /** Destroy all spawned actors. */
    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void DestroySpawnedActors();

    /** Destroy the actor that was spawned for the given entry. */
    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void DestroySpawnedActorForEntry(int32 EntryIdx);

    /** Get the actor that was spawned for the given entry and fire 'OnGetSpawnedActor' event. */
    UFUNCTION(BlueprintPure, Category = "Spawner")
    AActor* GetSpawnedActorForEntry(int32 EntryIdx) const;

    /** Get the number of spawned actors. */
    UFUNCTION(BlueprintPure, Category = "Spawner")
    int32 GetSpawnedActorCount() const;

protected:

    /** Internal spawn function. */
    void InternalSpawnActor(int32 EntryIdx);

    /** Event processor for the 'OnActorSpawned' event. */
    UFUNCTION()
    void ProcessEvent_OnActorSpawned(AActor* ActorPtr);

    /** Event processor for the 'OnGetSpawnedActor' event. */
    UFUNCTION()
    void ProcessEvent_OnGetSpawnedActor(AActor* ActorPtr);
};