// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicSpawner.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpawn, AActor*, ActorPtr);

USTRUCT(BlueprintType)
struct FLogicSpawnerEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    TSubclassOf<AActor> SpawnClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
    TObjectPtr<AActor> SpawnPoint;

    FLogicSpawnerEntry() :
        SpawnClass(TSubclassOf<AActor>()),
        SpawnPoint(nullptr)
    {}

    FLogicSpawnerEntry(UClass* InSpawnClass, AActor* InSpawnPoint) :
        SpawnClass(InSpawnClass),
        SpawnPoint(InSpawnPoint)
    {}
};

UCLASS()
class ACTORIO_API ALogicSpawner : public ALogicActorBase
{
    GENERATED_BODY()

public:

    ALogicSpawner();

public:

    UPROPERTY(EditInstanceOnly, Category = "Spawner")
    TArray<FLogicSpawnerEntry> ActorsToSpawn;

    UPROPERTY(EditInstanceOnly, Category = "Spawner")
    bool bSpawnOnStart;

    UPROPERTY(BlueprintAssignable, Category = "Spawner")
    FOnSpawn OnActorSpawned;

    UPROPERTY(BlueprintAssignable, Category = "Spawner")
    FSimpleActionDelegate OnSpawnFinished;

protected:

    UPROPERTY(Transient)
    TArray<TObjectPtr<AActor>> SpawnedActors;

public:

    virtual void BeginPlay() override;
    virtual void RegisterIOEvents_Implementation(FActorIOEventList& RegisteredEvents) override;
    virtual void RegisterIOFunctions_Implementation(FActorIOFunctionList& RegisteredFunctions) override;

public:

    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void Spawn();

    UFUNCTION(BlueprintPure, Category = "Spawner")
    AActor* GetSpawnedActorAt(int32 Index) const;

    UFUNCTION(BlueprintPure, Category = "Spawner")
    int32 GetSpawnedActorCount() const;

protected:

    UFUNCTION()
    void ProcessEvent_OnActorSpawned(AActor* ActorPtr);
};