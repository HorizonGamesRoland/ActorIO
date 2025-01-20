// Copyright 2025 Horizon Games. All Rights Reserved.

#include "ActorIOEditor.h"
#include "ActorIOEditorStyle.h"
#include "ActorIOAction.h"
#include "ActorIOComponent.h"
#include "ActorIOComponentVisualizer.h"
#include "ActorIOSystem.h"
#include "SActorIOEditor.h"
#include "LogicActors/LogicBranch.h"
#include "LogicActors/LogicCase.h"
#include "LogicActors/LogicCompare.h"
#include "LogicActors/LogicCondition.h"
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

#define LOCTEXT_NAMESPACE "ActorIOEditor"

FActorIOEditor& FActorIOEditor::Get()
{
	return FModuleManager::GetModuleChecked<FActorIOEditor>("ActorIOEditor");
}

void FActorIOEditor::StartupModule()
{
	// Initialize the editor style of the plugin.
	FActorIOEditorStyle::Initialize();

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TEXT("ActorIO"), FOnSpawnTab::CreateRaw(this, &FActorIOEditor::SpawnActorIOEditor))
		.SetDisplayName(LOCTEXT("TabName", "Actor I/O"))
		.SetTooltipText(LOCTEXT("TabTooltip", "Open the Actor I/O tab. Use this for level scripting."))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Event"));

	DelegateHandle_SelectionChange = USelection::SelectionChangedEvent.AddRaw(this, &FActorIOEditor::OnObjectSelectionChanged);
	DelegateHandle_DeleteActorsBegin = FEditorDelegates::OnDeleteActorsBegin.AddRaw(this, &FActorIOEditor::OnDeleteActorsBegin);

	if (GEditor)
	{
		GEditor->OnBlueprintCompiled().AddRaw(this, &FActorIOEditor::OnBlueprintCompiled);
	}

	if (GUnrealEd)
	{
		TSharedPtr<FComponentVisualizer> IOComponentVisualizer = MakeShared<FActorIOComponentVisualizer>();
		GUnrealEd->RegisterComponentVisualizer(UActorIOComponent::StaticClass()->GetFName(), IOComponentVisualizer);
		IOComponentVisualizer->OnRegister();
	}

	if (IPlacementModeModule::IsAvailable())
	{
		FPlacementCategoryInfo Info(LOCTEXT("ActorIOPlaceCategoryName", "Logic Actors"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Event"), "ActorIOPlaceCategory", TEXT("PMActorIOPlaceCategory"), 45);

		Info.ShortDisplayName = LOCTEXT("ActorIOPlaceCategoryShortName", "Logic");

		IPlacementModeModule& PlacementModeModule = IPlacementModeModule::Get();
		PlacementModeModule.RegisterPlacementCategory(Info);
		PlacementModeModule.RegisterPlaceableItem(Info.UniqueHandle, MakeShared<FPlaceableItem>(*UActorFactory::StaticClass(), FAssetData(ALogicBranch::StaticClass())));
		PlacementModeModule.RegisterPlaceableItem(Info.UniqueHandle, MakeShared<FPlaceableItem>(*UActorFactory::StaticClass(), FAssetData(ALogicCase::StaticClass())));
		PlacementModeModule.RegisterPlaceableItem(Info.UniqueHandle, MakeShared<FPlaceableItem>(*UActorFactory::StaticClass(), FAssetData(ALogicCompare::StaticClass())));
		PlacementModeModule.RegisterPlaceableItem(Info.UniqueHandle, MakeShared<FPlaceableItem>(*UActorFactory::StaticClass(), FAssetData(ALogicCondition::StaticClass())));
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
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TEXT("ActorIO"));

	USelection::SelectionChangedEvent.Remove(DelegateHandle_SelectionChange);
	FEditorDelegates::OnDeleteActorsBegin.Remove(DelegateHandle_DeleteActorsBegin);

	if (GEditor)
	{
		GEditor->OnBlueprintCompiled().Remove(DelegateHandle_BlueprintCompiled);
	}

	if (GUnrealEd)
	{
		GUnrealEd->UnregisterComponentVisualizer(UActorIOComponent::StaticClass()->GetFName());
	}

	if (IPlacementModeModule::IsAvailable())
	{
		IPlacementModeModule& PlacementModeModule = IPlacementModeModule::Get();
		PlacementModeModule.UnregisterPlacementCategory("ActorIOPlaceCategory");
	}

	FActorIOEditorStyle::Shutdown();
}

TSharedRef<SDockTab> FActorIOEditor::SpawnActorIOEditor(const FSpawnTabArgs& TabSpawnArgs)
{
	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	[
		SAssignNew(EditorWindow, SActorIOEditor)
	];

	const SDockTab::FOnTabClosedCallback TabClosedDelegate = SDockTab::FOnTabClosedCallback::CreateRaw(this, &FActorIOEditor::OnActorIOEditorClosed);
	SpawnedTab->SetOnTabClosed(TabClosedDelegate);

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
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		AActor* Actor = static_cast<AActor*>(*It);
		TArray<TWeakObjectPtr<UActorIOAction>> InputActions = UActorIOSystem::GetInputActionsForObject(Actor);
		for (int32 ActionIdx = 0; ActionIdx != InputActions.Num(); ++ActionIdx)
		{
			UActorIOAction* ActionPtr = InputActions[ActionIdx].Get();
			if (IsValid(ActionPtr))
			{
				ActionPtr->Modify();
			}
		}
	}
}

void FActorIOEditor::OnBlueprintCompiled()
{
	// A blueprint was recompiled, so the user may have exposed new I/O events or functions.
	// To make the changes appear immediately, we need to update the editor window.
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

UActorIOComponent* FActorIOEditor::AddIOComponenToActor(AActor* TargetActor, bool bSelectActor)
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

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FActorIOEditor, ActorIOEditor)