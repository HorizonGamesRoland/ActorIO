// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "LogicActors/LogicDataLayer.h"
#include "WorldPartition/DataLayer/DataLayerManager.h"
#include "WorldPartition/DataLayer/DataLayerAsset.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicDataLayer::ALogicDataLayer()
{
	DataLayerAsset = nullptr;
	bLoadRecursive = false;
}

void ALogicDataLayer::RegisterIOEvents(FActorIOEventList& EventRegistry)
{
	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicDataLayer::OnDataLayerLoaded"))
		.SetDisplayName(LOCTEXT("ALogicDataLayer.OnDataLayerLoaded", "OnDataLayerLoaded"))
		.SetTooltipText(LOCTEXT("ALogicDataLayer.OnDataLayerLoadedTooltip", "Event when the data layer is loaded and activated."))
		.SetMulticastDelegate(this, &OnDataLayerLoaded));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicDataLayer::OnDataLayerUnloaded"))
		.SetDisplayName(LOCTEXT("ALogicDataLayer.OnDataLayerUnloaded", "OnDataLayerUnloaded"))
		.SetTooltipText(LOCTEXT("ALogicDataLayer.OnDataLayerUnloadedTooltip", "Event when the data layer is unloaded."))
		.SetMulticastDelegate(this, &OnDataLayerUnloaded));
}

void ALogicDataLayer::RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry)
{
	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicDataLayer::LoadDataLayer"))
		.SetDisplayName(LOCTEXT("ALogicDataLayer.LoadDataLayer", "LoadDataLayer"))
		.SetTooltipText(LOCTEXT("ALogicDataLayer.LoadDataLayerTooltip", "Load the selected data layer. Fires 'OnDataLayerLoaded' once finished."))
		.SetFunction(TEXT("LoadDataLayer")));

	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicDataLayer::UnloadDataLayer"))
		.SetDisplayName(LOCTEXT("ALogicDataLayer.UnloadDataLayer", "UnloadDataLayer"))
		.SetTooltipText(LOCTEXT("ALogicDataLayer.UnloadDataLayerTooltip", "Unload the selected data layer. Fires 'OnDataLayerUnloaded' once finished."))
		.SetFunction(TEXT("UnloadDataLayer")));
}

void ALogicDataLayer::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UWorld* MyWorld = GetWorld();
	if (MyWorld && MyWorld->IsGameWorld())
	{
		UDataLayerManager* DataLayerManager = UDataLayerManager::GetDataLayerManager(MyWorld);
		if (DataLayerManager)
		{
			DataLayerManager->OnDataLayerInstanceRuntimeStateChanged.AddDynamic(this, &ThisClass::OnDataLayerLoadStateChanged);
		}
	}
}

void ALogicDataLayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UDataLayerManager* DataLayerManager = UDataLayerManager::GetDataLayerManager(GetWorld());
	if (DataLayerManager)
	{
		DataLayerManager->OnDataLayerInstanceRuntimeStateChanged.RemoveDynamic(this, &ThisClass::OnDataLayerLoadStateChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void ALogicDataLayer::LoadDataLayer()
{
	if (!DataLayerAsset)
	{
		// Do nothing if no data layer is selected.
		return;
	}

	UDataLayerManager* DataLayerManager = UDataLayerManager::GetDataLayerManager(GetWorld());
	if (DataLayerManager)
	{
		const UDataLayerInstance* DataLayerInstance = DataLayerManager->GetDataLayerInstanceFromAsset(DataLayerAsset);
		if (DataLayerInstance)
		{
			DataLayerManager->SetDataLayerInstanceRuntimeState(DataLayerInstance, EDataLayerRuntimeState::Activated, bLoadRecursive);
		}
	}
}

void ALogicDataLayer::UnloadDataLayer()
{
	if (!DataLayerAsset)
	{
		// Do nothing if no data layer is selected.
		return;
	}

	UDataLayerManager* DataLayerManager = UDataLayerManager::GetDataLayerManager(GetWorld());
	if (DataLayerManager)
	{
		const UDataLayerInstance* DataLayerInstance = DataLayerManager->GetDataLayerInstanceFromAsset(DataLayerAsset);
		if (DataLayerInstance)
		{
			DataLayerManager->SetDataLayerInstanceRuntimeState(DataLayerInstance, EDataLayerRuntimeState::Unloaded, bLoadRecursive);
		}
	}
}

bool ALogicDataLayer::IsDataLayerLoaded() const
{
	if (!DataLayerAsset)
	{
		// Do nothing if no data layer is selected.
		return false;
	}

	UDataLayerManager* DataLayerManager = UDataLayerManager::GetDataLayerManager(GetWorld());
	if (DataLayerManager)
	{
		const UDataLayerInstance* DataLayerInstance = DataLayerManager->GetDataLayerInstanceFromAsset(DataLayerAsset);
		if (DataLayerInstance)
		{
			EDataLayerRuntimeState CurrentState = DataLayerManager->GetDataLayerInstanceRuntimeState(DataLayerInstance);
			return CurrentState == EDataLayerRuntimeState::Activated;
		}
	}

	return false;
}

void ALogicDataLayer::OnDataLayerLoadStateChanged(const UDataLayerInstance* InDataLayer, EDataLayerRuntimeState InState)
{
	if (!DataLayerAsset)
	{
		// Do nothing if no data layer is selected.
		return;
	}

	UDataLayerManager* DataLayerManager = UDataLayerManager::GetDataLayerManager(GetWorld());
	if (DataLayerManager)
	{
		const UDataLayerInstance* DataLayerInstance = DataLayerManager->GetDataLayerInstanceFromAsset(DataLayerAsset);
		if (DataLayerInstance == InDataLayer)
		{
			if (InState == EDataLayerRuntimeState::Activated)
			{
				OnDataLayerLoaded.Broadcast();
			}
			else
			{
				OnDataLayerUnloaded.Broadcast();
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
