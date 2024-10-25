// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOEditorSubsystem.h"
#include "ActorIOEditor.h"
#include "Selection.h"
#include "GameFramework/Actor.h"
#include "Modules/ModuleManager.h"

void UActorIOEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	USelection::SelectionChangedEvent.AddUObject(this, &ThisClass::OnSelectionChanged);
}

void UActorIOEditorSubsystem::Deinitialize()
{
	USelection::SelectionChangedEvent.RemoveAll(this);
}

void UActorIOEditorSubsystem::OnSelectionChanged(UObject* NewSelection)
{
	USelection* SelectedActors = GEditor->GetSelectedActors();
	SelectedActor = SelectedActors ? SelectedActors->GetBottom<AActor>() : nullptr;

	FActorIOEditor& ActorIOEditorModule = FModuleManager::GetModuleChecked<FActorIOEditor>("ActorIOEditor");
	ActorIOEditorModule.UpdateEditorWindow();
}
