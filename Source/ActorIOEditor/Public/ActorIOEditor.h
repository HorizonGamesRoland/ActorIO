// Copyright 2024-2025 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "ActorIOPIEAuthorizer.h"
#include "EditorUndoClient.h"
#include "UObject/WeakObjectPtr.h"

class UActorIOComponent;
class SActorIOEditor;
class SDockTab;
class FSpawnTabArgs;
class AActor;

/**
 * Editor module implementation of the Actor I/O plugin.
 */
class FActorIOEditor : public IModuleInterface, public FEditorUndoClient
{
private:

	/** The editor widget inside the Actor I/O tab. */
	TSharedPtr<SActorIOEditor> EditorWindow;

	/** The currently selected actor in the level editor. */
	TWeakObjectPtr<AActor> SelectedActor;

	/** Handle for when a new object is selected in the editor. */
	FDelegateHandle DelegateHandle_SelectionChanged;

	/** Handle for when an actor is deleted in the level editor. */
	FDelegateHandle DelegateHandle_DeleteActorsBegin;

	/** Handle for when an actor is replaced in the level editor. */
	FDelegateHandle DelegateHandle_ActorReplaced;

	/** Handle for when a blueprint is compiled in the editor. */
	FDelegateHandle DelegateHandle_BlueprintCompiled;

	/** PIE authorizer to abort PIE sessions if the plugin is configured incorrectly. */
	FActorIOPIEAuthorizer PIEAuthorizer;

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
	
	/** Add an actor I/O component to the given actor. */
	UActorIOComponent* AddIOComponentToActor(AActor* TargetActor, bool bSelectActor);

private:

	/** Creates the Actor I/O editor tab. */
	TSharedRef<SDockTab> CreateActorIOEditorTab(const FSpawnTabArgs& TabSpawnArgs);

	/** Called when the Actor I/O tab is closed. */
	void OnActorIOEditorClosed(TSharedRef<SDockTab> DockTab);

	/** Called when a new object is selected in the editor. */
	void OnObjectSelectionChanged(UObject* NewSelection);

	/** Called when an actor is deleted in the level editor. */
	void OnDeleteActorsBegin();

	/** Called when an actor is replaced in the level editor. */
	void OnActorReplaced(AActor* OldActor, AActor* NewActor);

	/** Called when a blueprint is compiled in the editor. */
	void OnBlueprintCompiled();

public:

	//~ Begin FEditorUndoClient Interface
	virtual bool MatchesContext(const FTransactionContext& InContext, const TArray<TPair<UObject*, FTransactionObjectEvent>>& TransactionObjects) const override;
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;
	//~ End FEditorUndoClient Interface
};
