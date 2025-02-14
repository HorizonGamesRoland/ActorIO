// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO

#include "ActorIOEditor.h"
#include "ActorIOEditorStyle.h"
#include "ActorIOAction.h"
#include "ActorIOComponent.h"
#include "ActorIOComponentVisualizer.h"
#include "SActorIOEditor.h"
#include "LogicActors/LogicBranch.h"
#include "LogicActors/LogicCase.h"
#include "LogicActors/LogicCompare.h"
#include "LogicActors/LogicCounter.h"
#include "LogicActors/LogicGlobalEvent.h"
#include "LogicActors/LogicRelay.h"
#include "LogicActors/LogicSpawner.h"
#include "LogicActors/LogicTimeline.h"
#include "LogicActors/LogicTimer.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "GameFramework/Actor.h"
#include "IPlacementModeModule.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Selection.h"
#include "Editor.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
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
		PlacementModeModule.RegisterPlaceableItem(Info.UniqueHandle, MakeShared<FPlaceableItem>(*UActorFactory::StaticClass(), FAssetData(ALogicBranch::StaticClass())));
		PlacementModeModule.RegisterPlaceableItem(Info.UniqueHandle, MakeShared<FPlaceableItem>(*UActorFactory::StaticClass(), FAssetData(ALogicCase::StaticClass())));
		PlacementModeModule.RegisterPlaceableItem(Info.UniqueHandle, MakeShared<FPlaceableItem>(*UActorFactory::StaticClass(), FAssetData(ALogicCompare::StaticClass())));
		PlacementModeModule.RegisterPlaceableItem(Info.UniqueHandle, MakeShared<FPlaceableItem>(*UActorFactory::StaticClass(), FAssetData(ALogicCounter::StaticClass())));
		PlacementModeModule.RegisterPlaceableItem(Info.UniqueHandle, MakeShared<FPlaceableItem>(*UActorFactory::StaticClass(), FAssetData(ALogicGlobalEvent::StaticClass())));
		PlacementModeModule.RegisterPlaceableItem(Info.UniqueHandle, MakeShared<FPlaceableItem>(*UActorFactory::StaticClass(), FAssetData(ALogicRelay::StaticClass())));
		PlacementModeModule.RegisterPlaceableItem(Info.UniqueHandle, MakeShared<FPlaceableItem>(*UActorFactory::StaticClass(), FAssetData(ALogicSpawner::StaticClass())));
		PlacementModeModule.RegisterPlaceableItem(Info.UniqueHandle, MakeShared<FPlaceableItem>(*UActorFactory::StaticClass(), FAssetData(ALogicTimeline::StaticClass())));
		PlacementModeModule.RegisterPlaceableItem(Info.UniqueHandle, MakeShared<FPlaceableItem>(*UActorFactory::StaticClass(), FAssetData(ALogicTimer::StaticClass())));
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
		PlacementModeModule.UnregisterPlacementCategory("ActorIOPlaceCategory");
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
		SAssignNew(EditorWindow, SActorIOEditor)
	];

	const SDockTab::FOnTabClosedCallback TabClosedDelegate = SDockTab::FOnTabClosedCallback::CreateRaw(this, &FActorIOEditor::OnActorIOEditorClosed);
	SpawnedTab->SetOnTabClosed(TabClosedDelegate);

	// Refresh the editor window immediately.
	UpdateEditorWindow();

	return SpawnedTab;
}

void FActorIOEditor::UpdateEditorWindow()
{
	if (EditorWindow.IsValid())
	{
		EditorWindow->Refresh();
	}
}

void FActorIOEditor::OnActorIOEditorClosed(TSharedRef<SDockTab> DockTab)
{
	EditorWindow.Reset();
}

void FActorIOEditor::OnObjectSelectionChanged(UObject* NewSelection)
{
	USelection* SelectedActors = GEditor->GetSelectedActors();
	SelectedActor = SelectedActors->GetBottom<AActor>();

	UpdateEditorWindow();
}

void FActorIOEditor::OnDeleteActorsBegin()
{
	// Modify all actions who's caller is about to deleted for proper undo/redo support.
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

void FActorIOEditor::OnBlueprintCompiled()
{
	// A blueprint was recompiled, so the user may have exposed new I/O events or functions.
	// To make the changes appear immediately, we need to update the editor window.
	// This also handles the case where no I/O stuff was being exposed due to an error in the blueprint which may have got fixed with this recompile.
	UpdateEditorWindow();
}

SActorIOEditor* FActorIOEditor::GetEditorWindow() const
{
	return EditorWindow.Get();
}

AActor* FActorIOEditor::GetSelectedActor() const
{
	return SelectedActor.Get();
}

UActorIOComponent* FActorIOEditor::AddIOComponentToActor(AActor* TargetActor, bool bSelectActor)
{
	// Modify the actor to support undo/redo.
	// The transaction is already active at this point.
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
	if (bSuccess && EditorWindow.IsValid())
	{
		EditorWindow->SetViewInputActions(true);
	}
}

void FActorIOEditor::PostRedo(bool bSuccess)
{
	// We are redoing a 'ViewIOAction' transaction.
	// Do the opposite of PostUndo.
	if (bSuccess && EditorWindow.IsValid())
	{
		EditorWindow->SetViewInputActions(false);
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FActorIOEditor, ActorIOEditor)
