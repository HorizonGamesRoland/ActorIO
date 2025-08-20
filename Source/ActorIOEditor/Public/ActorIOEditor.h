// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ActorIOPIEAuthorizer.h"
#include "Misc/Optional.h"

class SActorIOEditor;
class SDockTab;
class FSpawnTabArgs;
class FPlacementModeID;

/**
 * Editor module implementation of the Actor I/O plugin.
 */
class FActorIOEditor : public IModuleInterface
{
private:

	/** The editor widget inside the Actor I/O tab. */
	TSharedPtr<SActorIOEditor> EditorWidget;

	/** PIE authorizer to abort PIE sessions if the plugin is configured incorrectly. */
	FActorIOPIEAuthorizer PIEAuthorizer;

	/** List of actors that we have added to the placement module. */
	TArray<TOptional<FPlacementModeID>> PlaceActors;

public:

	/** @return The editor module of the Actor I/O plugin. */
	static FActorIOEditor& Get();

	/** Request an update with the editor widget. */
	void UpdateEditorWidget();

	/** @return The editor widget inside the Actor I/O tab. */
	SActorIOEditor* GetEditorWidget() const;

private:

	/** Creates the Actor I/O editor tab. */
	TSharedRef<SDockTab> CreateActorIOEditorTab(const FSpawnTabArgs& TabSpawnArgs);

	/** Called when the Actor I/O tab is closed. */
	void OnActorIOEditorClosed(TSharedRef<SDockTab> DockTab);

	/** Called when a placement category is refreshed in the editor. */
	void OnPlacementModeCategoryRefreshed(FName CategoryName);

public:

	//~ Begin IModuleInterface Interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface Interface
};
