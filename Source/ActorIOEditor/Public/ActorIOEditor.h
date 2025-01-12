// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class SActorIOEditor;
class SDockTab;

class FActorIOEditor : public IModuleInterface
{
private:

	TSharedPtr<SActorIOEditor> EditorWindow;

	TWeakObjectPtr<AActor> SelectedActor;

	FDelegateHandle DelegateHandle_SelectionChange;

	FDelegateHandle DelegateHandle_DeleteActorsBegin;

public:

	/** Register the Actor IO editor module with the engine. */
	virtual void StartupModule() override;

	/** Unregister the Actor IO editor module. */
	virtual void ShutdownModule() override;

	/** @return The editor module of the Actor IO plugin. */
	static FActorIOEditor& Get();

	/** Request an update with the editor widget. */
	void UpdateEditorWindow();

	/** @return The editor widget inside the Actor IO tab. */
	SActorIOEditor* GetEditorWindow() const;

	/** @return The actor that is currently selected in the editor. */
	AActor* GetSelectedActor() const;

private:

	TSharedRef<SDockTab> SpawnActorIOEditor(const FSpawnTabArgs& TabSpawnArgs);

	void OnActorIOEditorClosed(TSharedRef<SDockTab> DockTab);

	void OnObjectSelectionChanged(UObject* NewSelection);

	void OnDeleteActorsBegin();
};
