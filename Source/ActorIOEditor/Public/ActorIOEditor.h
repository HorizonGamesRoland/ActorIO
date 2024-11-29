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

public:

	/** Register the Actor IO editor module with the engine. */
	virtual void StartupModule() override;

	/** Unregister the Actor IO editor module. */
	virtual void ShutdownModule() override;

	/** Request an update with the editor widget. */
	void UpdateEditorWindow();

	/** @return The editor widget inside the Actor IO tab. */
	SActorIOEditor* GetEditorWindow() const;

private:

	TSharedRef<SDockTab> SpawnActorIOEditor(const FSpawnTabArgs& TabSpawnArgs);

	void OnActorIOEditorClosed(TSharedRef<SDockTab> DockTab);
};
