// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class SActorIOPanel;
class SDockTab;

class FActorIOEditor : public IModuleInterface
{
private:

	TSharedPtr<SActorIOPanel> ActorIOPanel;

public:

	/** Register the ActorIO editor module with the engine. */
	virtual void StartupModule() override;

	/** Unregister the ActorIO editor module. */
	virtual void ShutdownModule() override;

	void UpdateActorIOPanel();

	/** @return The editor widget inside the Actor IO tab. */
	SActorIOPanel* GetActorIOPanel() const;

private:

	TSharedRef<SDockTab> SpawnTab_ActorIO(const FSpawnTabArgs& TabSpawnArgs);

	void OnActorIOEditorClosed(TSharedRef<SDockTab> DockTab);
};
