// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "LogicActors/LogicDataLayer.h"
#include "WorldPartition/DataLayer/DataLayerManager.h"
#include "WorldPartition/DataLayer/DataLayerAsset.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicDataLayer::ALogicDataLayer()
{
	DataLayerAsset = nullptr;
	bLoadRecursive = false;
	bIsLoaded = false;

#if WITH_EDITORONLY_DATA
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/ActorIO/AssetIcons/S_DataLayer"));
	if (SpriteComponent && SpriteTexture.Succeeded())
	{
		SpriteComponent->SetSprite(SpriteTexture.Get());
		SpriteComponent->SetRelativeScale3D_Direct(FVector(1.0f));
	}
#endif
}

void ALogicDataLayer::RegisterIOEvents(FActorIOEventList& EventRegistry)
{
	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicDataLayer::OnDataLayerLoaded"))
		.SetDisplayName(LOCTEXT("ALogicDataLayer.OnDataLayerLoaded", "OnDataLayerLoaded"))
		.SetTooltipText(LOCTEXT("ALogicDataLayer.OnDataLayerLoadedTooltip", "Event when the data layer is loaded. Note that this only means the data layer is now active, but its actors are NOT streamed in yet!"))
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
		.SetTooltipText(LOCTEXT("ALogicDataLayer.LoadDataLayerTooltip", "Load the selected data layer, and activate it. Fires 'OnDataLayerLoaded' once finished."))
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
			bIsLoaded = CheckDataLayerLoadState(bLoadRecursive);
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
	const UDataLayerInstance* DataLayerInstance = DataLayerManager ? DataLayerManager->GetDataLayerInstanceFromAsset(DataLayerAsset) : nullptr;
	if (DataLayerInstance)
	{
		DataLayerManager->SetDataLayerInstanceRuntimeState(DataLayerInstance, EDataLayerRuntimeState::Activated, bLoadRecursive);
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
	const UDataLayerInstance* DataLayerInstance = DataLayerManager ? DataLayerManager->GetDataLayerInstanceFromAsset(DataLayerAsset) : nullptr;
	if (DataLayerInstance)
	{
		DataLayerManager->SetDataLayerInstanceRuntimeState(DataLayerInstance, EDataLayerRuntimeState::Unloaded, bLoadRecursive);
	}
}

bool ALogicDataLayer::CheckDataLayerLoadState(bool bIncludeChildren) const
{
	if (!DataLayerAsset)
	{
		// Do nothing if no data layer is selected.
		return false;
	}

	UDataLayerManager* DataLayerManager = UDataLayerManager::GetDataLayerManager(GetWorld());
	const UDataLayerInstance* DataLayerInstance = DataLayerManager ? DataLayerManager->GetDataLayerInstanceFromAsset(DataLayerAsset) : nullptr;
	if (!DataLayerInstance || DataLayerInstance->GetEffectiveRuntimeState() != EDataLayerRuntimeState::Activated)
	{
		return false;
	}

	if (bIncludeChildren)
	{
		for (const UDataLayerInstance* ChildDataLayer : DataLayerInstance->GetChildren())
		{
			if (ChildDataLayer->GetEffectiveRuntimeState() != EDataLayerRuntimeState::Activated)
			{
				return false;
			}
		}
	}

	return true;
}

void ALogicDataLayer::OnDataLayerLoadStateChanged(const UDataLayerInstance* InDataLayer, EDataLayerRuntimeState InState)
{
	const bool bPreviousLoadState = bIsLoaded;
	bIsLoaded = CheckDataLayerLoadState(bLoadRecursive);
	
	if (bIsLoaded != bPreviousLoadState)
	{
		if (bIsLoaded)
		{
			OnDataLayerLoaded.Broadcast();
		}
		else
		{
			OnDataLayerUnloaded.Broadcast();
		}
	}
}

#undef LOCTEXT_NAMESPACE
