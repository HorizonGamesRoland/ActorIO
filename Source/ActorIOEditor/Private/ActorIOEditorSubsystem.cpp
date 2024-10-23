// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOEditorSubsystem.h"
#include "ActorIOEditor.h"
#include "Selection.h"
#include "GameFramework/Actor.h"
#include "Modules/ModuleManager.h"

void UActorIOEditorSubsystem::Tick(float DeltaTime)
{
	USelection* SelectedActors = GEditor->GetSelectedActors();
	SelectedActor = SelectedActors ? SelectedActors->GetBottom<AActor>() : nullptr;

	FActorIOEditor& ActorIOEditorModule = FModuleManager::GetModuleChecked<FActorIOEditor>("ActorIOEditor");
	ActorIOEditorModule.UpdateActorIOPanel();
}

TStatId UActorIOEditorSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FActorIOEditorSubsystem, STATGROUP_Tickables);
}
