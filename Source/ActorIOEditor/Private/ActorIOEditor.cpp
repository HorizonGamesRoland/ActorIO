// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOEditor.h"
#include "ActorIOEditorStyle.h"
#include "ActorIOAction.h"
#include "ActorIOComponentVisualizer.h"
#include "LogicActors/LogicActorBase.h"
#include "Widgets/SActorIOEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"
#include "Engine/Blueprint.h"
#include "IPlacementModeModule.h"
#include "Features/IModularFeatures.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Editor.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Misc/ITransaction.h"
#include "Misc/EngineVersionComparison.h"

#define LOCTEXT_NAMESPACE "ActorIOEditor"

FActorIOEditor& FActorIOEditor::Get()
{
	return FModuleManager::GetModuleChecked<FActorIOEditor>("ActorIOEditor");
}

void FActorIOEditor::StartupModule()
{
	// Initialize the editor style of the plugin.
	FActorIOEditorStyle::Initialize();

	// Register Actor I/O editor tab.
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TEXT("ActorIO"), FOnSpawnTab::CreateRaw(this, &FActorIOEditor::CreateActorIOEditorTab))
		.SetDisplayName(LOCTEXT("TabName", "Actor I/O"))
		.SetTooltipText(LOCTEXT("TabTooltip", "Open the Actor I/O editor tab to edit scripted actions of actors in the level."))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Event"));

	// Register undo client.
	GEditor->RegisterForUndo(this);

	// Register component visualizer to draw I/O lines between actors.
	TSharedPtr<FComponentVisualizer> IOComponentVisualizer = MakeShared<FActorIOComponentVisualizer>();
	GUnrealEd->RegisterComponentVisualizer(TEXT("ActorIOComponent"), IOComponentVisualizer);
	IOComponentVisualizer->OnRegister();

	// Register PIE authorizer to abort PIE sessions if the plugin is configured incorrectly.
	// #NOTE: IModularFeatures won't compile in IWYU mode.
	PIEAuthorizer = FActorIOPIEAuthorizer();
	IModularFeatures::Get().RegisterModularFeature(FActorIOPIEAuthorizer::GetModularFeatureName(), &PIEAuthorizer);

	// Register custom placement mode.
	if (IPlacementModeModule::IsAvailable())
	{
		FPlacementCategoryInfo Info(LOCTEXT("ActorIOPlaceCategoryName", "Logic Actors"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Event"), TEXT("ActorIOPlaceCategory"), TEXT("PMActorIOPlaceCategory"), 25);

#if UE_VERSION_NEWER_THAN(5, 5, 0)
		Info.ShortDisplayName = LOCTEXT("ActorIOPlaceCategoryShortName", "Logic");
#endif

		IPlacementModeModule& PlacementModeModule = IPlacementModeModule::Get();
		PlacementModeModule.RegisterPlacementCategory(Info);
		PlacementModeModule.OnPlacementModeCategoryRefreshed().AddRaw(this, &FActorIOEditor::OnPlacementModeCategoryRefreshed);
	}
}

void FActorIOEditor::ShutdownModule()
{
	// Unregister Actor I/O editor tab.
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TEXT("ActorIO"));

	if (GEditor)
	{
		// Unregister undo client.
		GEditor->UnregisterForUndo(this);
	}

	// Unregister component visualizer.
	if (GUnrealEd)
	{
		GUnrealEd->UnregisterComponentVisualizer(TEXT("ActorIOComponent"));
	}

	// Unregister PIE authorizer.
	IModularFeatures::Get().UnregisterModularFeature(FActorIOPIEAuthorizer::GetModularFeatureName(), &PIEAuthorizer);

	// Unregister custom placement mode.
	if (IPlacementModeModule::IsAvailable())
	{
		IPlacementModeModule& PlacementModeModule = IPlacementModeModule::Get();
		PlacementModeModule.UnregisterPlacementCategory(TEXT("ActorIOPlaceCategory"));
	}

	// Unregister the editor style of the plugin.
	FActorIOEditorStyle::Shutdown();
}

TSharedRef<SDockTab> FActorIOEditor::CreateActorIOEditorTab(const FSpawnTabArgs& TabSpawnArgs)
{
	// Reset action list column sizes to defaults before creating the tab.
	FActorIOEditorStyle::SetupActionListColumnSizes();

	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	.OnTabClosed(SDockTab::FOnTabClosedCallback::CreateRaw(this, &FActorIOEditor::OnActorIOEditorClosed))
	[
		SAssignNew(EditorWidget, SActorIOEditor)
	];

	// Refresh the editor window immediately.
	UpdateEditorWidget();

	return SpawnedTab;
}

void FActorIOEditor::UpdateEditorWidget()
{
	if (EditorWidget.IsValid())
	{
		EditorWidget->Refresh();
	}
}

void FActorIOEditor::OnActorIOEditorClosed(TSharedRef<SDockTab> DockTab)
{
	EditorWidget.Reset();
}

void FActorIOEditor::OnPlacementModeCategoryRefreshed(FName CategoryName)
{
	IPlacementModeModule& PlacementModeModule = IPlacementModeModule::Get();

	if (CategoryName == TEXT("ActorIOPlaceCategory"))
	{
		// Refresh "AllClasses" category to ensure that we have the proper items, since we are pulling the data from that category.
		PlacementModeModule.RegenerateItemsForCategory(FBuiltInPlacementCategories::AllClasses());
	}
	else if (CategoryName == FBuiltInPlacementCategories::AllClasses())
	{
		// Unregister all previous placeable items.
		if (!PlaceActors.IsEmpty())
		{
			for (TOptional<FPlacementModeID>& PlaceActor : PlaceActors)
			{
				if (PlaceActor.IsSet())
				{
					PlacementModeModule.UnregisterPlaceableItem(PlaceActor.GetValue());
				}
			}

			PlaceActors.Empty();
		}

		TArray<TSharedPtr<FPlaceableItem>> PlaceableItems;
		PlacementModeModule.GetItemsForCategory(FBuiltInPlacementCategories::AllClasses(), PlaceableItems);

		// Register new placeable items for logic actors.
		// This includes blueprints as well.
		for (TSharedPtr<FPlaceableItem>& PlaceableItem : PlaceableItems)
		{
			UClass* PlaceableItemClass = nullptr;

			UObject* PlaceableAsset = PlaceableItem->AssetData.GetAsset();
			if (PlaceableAsset)
			{
				// If the asset is a native class, then we just need to cast.
				if (PlaceableAsset->GetClass()->IsChildOf<UClass>())
				{
					PlaceableItemClass = Cast<UClass>(PlaceableAsset);
				}
				// Otherwise if its a blueprint asset, infer from BP parent class.
				else if (PlaceableAsset->GetClass()->IsChildOf<UBlueprint>())
				{
					UBlueprint* PlaceableItemBlueprint = Cast<UBlueprint>(PlaceableAsset);
					PlaceableItemClass = PlaceableItemBlueprint->ParentClass.Get();
				}
			}

			// Register a new entry from the existing data in "All Classes".
			if (PlaceableItemClass && PlaceableItemClass->IsChildOf<ALogicActorBase>())
			{
				TSharedRef<FPlaceableItem> NewPlaceable = MakeShared<FPlaceableItem>(PlaceableItem->AssetFactory, PlaceableItem->AssetData);
				PlaceActors.Add(PlacementModeModule.RegisterPlaceableItem(TEXT("ActorIOPlaceCategory"), NewPlaceable));
			}
		}
	}
}

SActorIOEditor* FActorIOEditor::GetEditorWidget() const
{
	return EditorWidget.Get();
}

bool FActorIOEditor::MatchesContext(const FTransactionContext& InContext, const TArray<TPair<UObject*, FTransactionObjectEvent>>& TransactionObjects) const
{
	// Ensure that we only react to a very specific transaction called 'ViewIOAction'.
	// For more info see PostUndo below.
	return InContext.Context == TEXT("ViewIOAction");
}

void FActorIOEditor::PostUndo(bool bSuccess)
{
	// We are undoing a 'ViewIOAction' transaction.
	// This means the user was viewing input actions, and clicked on one an action's view button.
	// This resulted in the actor being selected and the I/O editor switching to outputs tab.
	// Since the 'bViewInputActions' param is not UPROPERTY we need to manually revert it.
	if (bSuccess && EditorWidget.IsValid())
	{
		EditorWidget->SetViewInputActions(true);
	}
}

void FActorIOEditor::PostRedo(bool bSuccess)
{
	// We are redoing a 'ViewIOAction' transaction.
	// Do the opposite of PostUndo.
	if (bSuccess && EditorWidget.IsValid())
	{
		EditorWidget->SetViewInputActions(false);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FActorIOEditor, ActorIOEditor)
