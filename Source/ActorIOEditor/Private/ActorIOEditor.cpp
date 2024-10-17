// Copyright 2024 Horizon Games. All Rights Reserved.

#include "ActorIOEditor.h"
#include "ActorIOEditorStyle.h"
#include "SActorIOPanel.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "FActorIOEditor"

void FActorIOEditor::StartupModule()
{
	// Initialize the editor style of the plugin.
	FActorIOEditorStyle::Initialize();
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TEXT("ActorIO"), FOnSpawnTab::CreateRaw(this, &FActorIOEditor::SpawnTab_ActorIO))
		.SetDisplayName(FText::FromString("Actor IO"))
		.SetTooltipText(FText::FromString("Open the Actor IO tab. Use this for level scripting."))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Event"));
}

void FActorIOEditor::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TEXT("ActorIO"));

	FActorIOEditorStyle::Shutdown();
}

void FActorIOEditor::UpdateActorIOPanel(AActor* InActor)
{
	if (ActorIOPanel.IsValid())
	{
		ActorIOPanel->RebuildFromState(InActor);
	}
}

TSharedRef<SDockTab> FActorIOEditor::SpawnTab_ActorIO(const FSpawnTabArgs& TabSpawnArgs)
{
	TSharedRef<SDockTab> SpawnedTab = SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	[
		SAssignNew(ActorIOPanel, SActorIOPanel)
	];

	const SDockTab::FOnTabClosedCallback TabClosedDelegate = SDockTab::FOnTabClosedCallback::CreateRaw(this, &FActorIOEditor::OnActorIOEditorClosed);
	SpawnedTab->SetOnTabClosed(TabClosedDelegate);

	return SpawnedTab;
}

void FActorIOEditor::OnActorIOEditorClosed(TSharedRef<SDockTab> DockTab)
{
	ActorIOPanel.Reset();
}

SActorIOPanel* FActorIOEditor::GetActorIOPanel() const
{
	return ActorIOPanel.Get();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FActorIOEditor, ActorIOEditor)