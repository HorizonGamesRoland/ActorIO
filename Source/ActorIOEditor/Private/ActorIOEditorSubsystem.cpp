// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOEditorSubsystem.h"
#include "ActorIOEditor.h"
#include "ActorIOComponent.h"
#include "ActorIOAction.h"
#include "GameFramework/Actor.h"
#include "DataLayer/DataLayerEditorSubsystem.h"
#include "Selection.h"
#include "Editor.h"
#include "Containers/Ticker.h"
#include "Misc/EngineVersionComparison.h"

UActorIOEditorSubsystem* UActorIOEditorSubsystem::Get()
{
	return GEditor->GetEditorSubsystem<UActorIOEditorSubsystem>();
}

void UActorIOEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	DelegateHandle_SelectionChanged = USelection::SelectionChangedEvent.AddUObject(this, &ThisClass::OnObjectSelectionChanged);
	DelegateHandle_DeleteActorsBegin = FEditorDelegates::OnDeleteActorsBegin.AddUObject(this, &ThisClass::OnDeleteOrCutActorsBegin);
	DelegateHandle_CutActorsBegin = FEditorDelegates::OnEditCutActorsBegin.AddUObject(this, &ThisClass::OnDeleteOrCutActorsBegin);
#if UE_VERSION_NEWER_THAN(5, 5, 0)
	DelegateHandle_ActorReplaced = FEditorDelegates::OnEditorActorReplaced.AddUObject(this, &ThisClass::OnActorReplaced);
#endif

	UDataLayerEditorSubsystem* DataLayerSubsystem = Collection.InitializeDependency<UDataLayerEditorSubsystem>();
	if (DataLayerSubsystem)
	{
		DelegateHandle_DataLayersEditorLoadStateChanged =
			DataLayerSubsystem->OnActorDataLayersEditorLoadingStateChanged().AddUObject(this, &ThisClass::OnDataLayersEditorLoadStateChanged);
	}

	DelegateHandle_BlueprintCompiled = GEditor->OnBlueprintCompiled().AddUObject(this, &ThisClass::OnBlueprintCompiled);
}

void UActorIOEditorSubsystem::Deinitialize()
{
	USelection::SelectionChangedEvent.Remove(DelegateHandle_SelectionChanged);
	FEditorDelegates::OnDeleteActorsBegin.Remove(DelegateHandle_DeleteActorsBegin);
	FEditorDelegates::OnEditCutActorsBegin.Remove(DelegateHandle_CutActorsBegin);
#if UE_VERSION_NEWER_THAN(5, 5, 0)
	FEditorDelegates::OnEditorActorReplaced.Remove(DelegateHandle_ActorReplaced);
#endif

	UDataLayerEditorSubsystem* DataLayerSubsystem = GEditor->GetEditorSubsystem<UDataLayerEditorSubsystem>();
	if (DataLayerSubsystem)
	{
		DataLayerSubsystem->OnActorDataLayersEditorLoadingStateChanged().Remove(DelegateHandle_DataLayersEditorLoadStateChanged);
	}

	GEditor->OnBlueprintCompiled().Remove(DelegateHandle_BlueprintCompiled);
}

AActor* UActorIOEditorSubsystem::GetSelectedActor() const
{
	return SelectedActor.Get();
}

UActorIOComponent* UActorIOEditorSubsystem::AddIOComponentToActor(AActor* TargetActor, bool bSelectActor)
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
	if (bSelectActor)
	{
		GEditor->SelectNone(true, false, false);
		GEditor->SelectActor(TargetActor, true, true);
	}

	return NewComponent;
}

void UActorIOEditorSubsystem::OnObjectSelectionChanged(UObject* NewSelection)
{
	USelection* SelectedActors = GEditor->GetSelectedActors();
	SelectedActor = SelectedActors->GetBottom<AActor>();

	FActorIOEditor& ActorIOEditor = FActorIOEditor::Get();
	ActorIOEditor.UpdateEditorWidget();
}

void UActorIOEditorSubsystem::OnDeleteOrCutActorsBegin()
{
	// Modify all actions who's caller is about to be deleted.
	// The transaction is already active at this point.
	for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
	{
		AActor* Actor = static_cast<AActor*>(*It);
		for (UActorIOAction* InputAction : IActorIO::GetInputActionsForObject(Actor))
		{
			InputAction->Modify();
			InputAction->TargetActor = nullptr;
		}
	}
}

void UActorIOEditorSubsystem::OnActorReplaced(AActor* OldActor, AActor* NewActor)
{
	// Modify all actions that point to the old actor.
	// The transaction is already active at this point.
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

void UActorIOEditorSubsystem::OnDataLayersEditorLoadStateChanged(bool bIsFromUserChange)
{
	// Update the I/O editor to catch case where one or more currrently visible actions' target actor gets unloaded with the data layer.
	// Additional one frame delay is needed for the target actor to become truly invalid.
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([](float DeltaTime)
	{
		FActorIOEditor& ActorIOEditor = FActorIOEditor::Get();
		ActorIOEditor.UpdateEditorWidget();

		return false; // stops the timer
	}));
}

void UActorIOEditorSubsystem::OnBlueprintCompiled()
{
	// A blueprint was recompiled, so the user may have exposed new I/O events or functions.
	// To make the changes appear immediately, we need to update the editor window.
	// This also handles the case where no I/O stuff was being exposed due to an error in the blueprint which may have got fixed with this recompile.
	FActorIOEditor& ActorIOEditor = FActorIOEditor::Get();
	ActorIOEditor.UpdateEditorWidget();
}
