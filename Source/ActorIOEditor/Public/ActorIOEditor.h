// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class SActorIOEditor;
class SDockTab;

/**
 * Editor module implementation of the Actor I/O plugin.
 */
class FActorIOEditor : public IModuleInterface
{
private:

	/** The editor widget inside the Actor I/O tab. */
	TSharedPtr<SActorIOEditor> EditorWindow;

	/** The currently selected actor in the level editor. */
	TWeakObjectPtr<AActor> SelectedActor;

	/** Handle for when a new object is selected in the editor. */
	FDelegateHandle DelegateHandle_SelectionChange;

	/** Handle for when an actor is deleted in the level editor. */
	FDelegateHandle DelegateHandle_DeleteActorsBegin;

public:

	/** Register the Actor I/O editor module with the engine. */
	virtual void StartupModule() override;

	/** Unregister the Actor I/O editor module. */
	virtual void ShutdownModule() override;

	/** @return The editor module of the Actor I/O plugin. */
	static FActorIOEditor& Get();

	/** Request an update with the editor widget. */
	void UpdateEditorWindow();

	/** @return The editor widget inside the Actor I/O tab. */
	SActorIOEditor* GetEditorWindow() const;

	/** @return The actor that is currently selected in the editor. */
	AActor* GetSelectedActor() const;

private:

	/** Creates the Actor I/O editor tab. */
	TSharedRef<SDockTab> SpawnActorIOEditor(const FSpawnTabArgs& TabSpawnArgs);

	/** Called when the Actor I/O tab is closed. */
	void OnActorIOEditorClosed(TSharedRef<SDockTab> DockTab);

	/** Called when a new object is selected in the editor. */
	void OnObjectSelectionChanged(UObject* NewSelection);

	/** Called when an actor is deleted in the level editor. */
	void OnDeleteActorsBegin();
};
