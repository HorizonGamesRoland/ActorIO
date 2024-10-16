// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOEditor.h"
#include "ActorIOEditorStyle.h"
#include "SActorIOPanel.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FActorIOEditorModule"

void FActorIOEditorModule::StartupModule()
{
	// Initialize the editor style of the plugin.
	FActorIOEditorStyle::Initialize();

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TEXT("ActorIO"), FOnSpawnTab::CreateRaw(this, &FActorIOEditorModule::SpawnTab))
		.SetGroup(FWorkspaceItem::NewGroup(FText::FromString("Menu Root")))
		.SetDisplayName(FText::FromString("Actor IO"))
		.SetTooltipText(FText::FromString("Actor IO Window"));
}

void FActorIOEditorModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TEXT("ActorIO"));
}

TSharedRef<SDockTab> FActorIOEditorModule::SpawnTab(const FSpawnTabArgs& TabSpawnArgs)
{
	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	[
		SNew(SActorIOPanel)
		.Tool(SharedThis(this))
	];

	return SpawnedTab;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FActorIOEditorModule, ActorIOEditor)