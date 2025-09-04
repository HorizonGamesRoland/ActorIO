// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "ActorIO.h"
#include "LogicActors/LogicActorBase.h"
#include "LogicDataLayer.generated.h"

class UDataLayerAsset;
class UDataLayerInstance;
enum class EDataLayerRuntimeState : uint8;

/**
 * An actor that handles the loading and unloading of a world partition data layer.
 */
UCLASS()
class ACTORIO_API ALogicDataLayer : public ALogicActorBase
{
	GENERATED_BODY()

public:

    /** Default constructor. */
    ALogicDataLayer();

public:

    /** The data layer to load or unload. */
    UPROPERTY(EditInstanceOnly, Category = "Data Layer Settings")
    TObjectPtr<UDataLayerAsset> DataLayerAsset;

    /** Whether data layer loading should also apply to child data layers. */
    UPROPERTY(EditInstanceOnly, Category = "Data Layer Settings")
    bool bLoadRecursive;

    /** Event when the data layer is loaded. Note that this only means the data layer is now active, but its actors are NOT streamed in yet! */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FSimpleActionDelegate OnDataLayerLoaded;

    /** Event when the data layer is unloaded. */
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FSimpleActionDelegate OnDataLayerUnloaded;

private:

    /** The currently known load state of the selected data layer. */
    bool bIsLoaded;

public:

    //~ Begin ALogicActorBase Interface
    virtual void RegisterIOEvents(FActorIOEventList& EventRegistry) override;
    virtual void RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    //~ End ALogicActorBase Interface

public:

    /** Load the selected data layer, and activate it. Fires 'OnDataLayerLoaded' once finished. */
    UFUNCTION(BlueprintCallable, Category = "LogicActors|LogicDataLayer")
    void LoadDataLayer();

    /** Unload the selected data layer. Fires 'OnDataLayerUnloaded' once finished. */
    UFUNCTION(BlueprintCallable, Category = "LogicActors|LogicDataLayer")
    void UnloadDataLayer();

    /** Get the load state of the selected data layer. */
    UFUNCTION(BlueprintPure, Category = "LogicActors|LogicDataLayer")
    bool IsDataLayerLoaded() const { return bIsLoaded; }

protected:

    /** Recursively check the effective load state of the data layer and its childrens. */
    bool CheckDataLayerLoadState(const UDataLayerInstance* InDataLayer, bool bIncludeChildren) const;

    /** Called when a data layer is loaded or unloaded. */
    UFUNCTION()
    void OnDataLayerLoadStateChanged(const UDataLayerInstance* InDataLayer, EDataLayerRuntimeState InState);
};
