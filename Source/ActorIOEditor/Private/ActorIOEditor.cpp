// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOEditor.h"
#include "ActorIOEditorStyle.h"
#include "ActorIOAction.h"
#include "ActorIOComponent.h"
#include "ActorIOComponentVisualizer.h"
#include "LogicActors/LogicBranch.h"
#include "LogicActors/LogicCase.h"
#include "LogicActors/LogicCompare.h"
#include "LogicActors/LogicCounter.h"
#include "LogicActors/LogicGlobalEvent.h"
#include "LogicActors/LogicRelay.h"
#include "LogicActors/LogicSpawner.h"
#include "LogicActors/LogicTimeline.h"
#include "LogicActors/LogicTimer.h"
#include "Widgets/SActorIOEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"
#include "GameFramework/Actor.h"
#include "Engine/Blueprint.h"
#include "IPlacementModeModule.h"
#include "Features/IModularFeatures.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Selection.h"
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
		.SetTooltipText(LOCTEXT("TabTooltip", "Open the Actor I/O tab. Use this for level scripting."))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Event"));

	if (GEditor)
	{
		// Bind delegates.
		DelegateHandle_SelectionChanged = USelection::SelectionChangedEvent.AddRaw(this, &FActorIOEditor::OnObjectSelectionChanged);
		DelegateHandle_DeleteActorsBegin = FEditorDelegates::OnDeleteActorsBegin.AddRaw(this, &FActorIOEditor::OnDeleteActorsBegin);
#if UE_VERSION_NEWER_THAN(5, 5, 0)
		DelegateHandle_ActorReplaced = FEditorDelegates::OnEditorActorReplaced.AddRaw(this, &FActorIOEditor::OnActorReplaced);
#endif
		DelegateHandle_BlueprintCompiled = GEditor->OnBlueprintCompiled().AddRaw(this, &FActorIOEditor::OnBlueprintCompiled);

		// Register undo client.
		GEditor->RegisterForUndo(this);
	}

	// Register component visualizer to draw I/O lines between actors.
	if (GUnrealEd)
	{
		TSharedPtr<FComponentVisualizer> IOComponentVisualizer = MakeShared<FActorIOComponentVisualizer>();
		GUnrealEd->RegisterComponentVisualizer(UActorIOComponent::StaticClass()->GetFName(), IOComponentVisualizer);
		IOComponentVisualizer->OnRegister();
	}

	// Register PIE authorizer to abort PIE sessions if the plugin is configured incorrectly.
	// #NOTE: IModularFeatures won't compile in IWYU mode.
	PIEAuthorizer = FActorIOPIEAuthorizer();
	IModularFeatures::Get().RegisterModularFeature(FActorIOPIEAuthorizer::GetModularFeatureName(), &PIEAuthorizer);

	// Register custom placement mode.
	if (IPlacementModeModule::IsAvailable())
	{
		FPlacementCategoryInfo Info(LOCTEXT("ActorIOPlaceCategoryName", "Logic Actors"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Event"), "ActorIOPlaceCategory", TEXT("PMActorIOPlaceCategory"), 45);

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
		// Clear delegates.
		USelection::SelectionChangedEvent.Remove(DelegateHandle_SelectionChanged);
		FEditorDelegates::OnDeleteActorsBegin.Remove(DelegateHandle_DeleteActorsBegin);
#if UE_VERSION_NEWER_THAN(5, 5, 0)
		FEditorDelegates::OnEditorActorReplaced.Remove(DelegateHandle_ActorReplaced);
#endif
		GEditor->OnBlueprintCompiled().Remove(DelegateHandle_BlueprintCompiled);

		// Unegister undo client.
		GEditor->UnregisterForUndo(this);
	}

	// Unregister component visualizer.
	if (GUnrealEd)
	{
		GUnrealEd->UnregisterComponentVisualizer(UActorIOComponent::StaticClass()->GetFName());
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
	[
		SAssignNew(EditorWidget, SActorIOEditor)
	];

	const SDockTab::FOnTabClosedCallback TabClosedDelegate = SDockTab::FOnTabClosedCallback::CreateRaw(this, &FActorIOEditor::OnActorIOEditorClosed);
	SpawnedTab->SetOnTabClosed(TabClosedDelegate);

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

void FActorIOEditor::OnObjectSelectionChanged(UObject* NewSelection)
{
	USelection* SelectedActors = GEditor->GetSelectedActors();
	SelectedActor = SelectedActors->GetBottom<AActor>();

	UpdateEditorWidget();
}

void FActorIOEditor::OnDeleteActorsBegin()
{
	// Modify all actions who's caller is about to be deleted for proper undo/redo support.
	// The transaction is already active at this point.
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		AActor* Actor = static_cast<AActor*>(*It);
		for (UActorIOAction* InputAction : IActorIO::GetInputActionsForObject(Actor))
		{
			InputAction->Modify();
		}
	}
}

void FActorIOEditor::OnActorReplaced(AActor* OldActor, AActor* NewActor)
{
	// Modify all actions that point to the old actor.
	const TArray<UActorIOAction*> InputActions = IActorIO::GetInputActionsForObject(OldActor);
	for (UActorIOAction* InputAction : InputActions)
	{
		InputAction->Modify();
		InputAction->TargetActor = NewActor;
	}

	// Auto add an IO component to the new actor if needed.
	if (OldActor->GetComponentByClass<UActorIOComponent>() || InputActions.Num() > 0)
	{
		if (!NewActor->GetComponentByClass<UActorIOComponent>())
		{
			AddIOComponentToActor(NewActor, false);
		}
	}
}

void FActorIOEditor::OnBlueprintCompiled()
{
	// A blueprint was recompiled, so the user may have exposed new I/O events or functions.
	// To make the changes appear immediately, we need to update the editor window.
	// This also handles the case where no I/O stuff was being exposed due to an error in the blueprint which may have got fixed with this recompile.
	UpdateEditorWidget();
}

void FActorIOEditor::OnPlacementModeCategoryRefreshed(FName CategoryName)
{
	if (CategoryName == FBuiltInPlacementCategories::AllClasses())
	{
		IPlacementModeModule& PlacementModeModule = IPlacementModeModule::Get();

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

AActor* FActorIOEditor::GetSelectedActor() const
{
	return SelectedActor.Get();
}

UActorIOComponent* FActorIOEditor::AddIOComponentToActor(AActor* TargetActor, bool bSelectActor)
{
	// Modify the actor to support undo/redo.
	// The transaction should already be active at this point.
	TargetActor->Modify();

	UActorIOComponent* NewComponent = NewObject<UActorIOComponent>(TargetActor, TEXT("ActorIOComponent"), RF_Transactional);
	NewComponent->OnComponentCreated();
	NewComponent->RegisterComponent();

	TargetActor->AddInstanceComponent(NewComponent);

	// Re-select the actor so that the render state is created for the I/O visualizer.
	// Also the component list of the actor gets updated.
	if (bSelectActor && GEditor)
	{
		GEditor->SelectNone(true, false, false);
		GEditor->SelectActor(TargetActor, true, true);
	}

	return NewComponent;
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
