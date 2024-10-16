// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FActorIOEditorModule : public IModuleInterface
{
public:

	/** Register the ActorIO editor module with the engine. */
	virtual void StartupModule() override;

	/** Unregister the ActorIO editor module. */
	virtual void ShutdownModule() override;

private:

	/** Register the plugin's settings window. */
	void RegisterSettings();

	/** Register the plugin's message log category. */
	void RegisterMessageLog();

	/** Unregister the plugin's settings window. */
	void UnRegisterSettings();

	/** Unregister the plugin's message log category. */
	void UnRegisterMessageLog();
};
