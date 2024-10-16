// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class SDockTab;

class FActorIOEditorModule : public IModuleInterface
{
public:

	/** Register the ActorIO editor module with the engine. */
	virtual void StartupModule() override;

	/** Unregister the ActorIO editor module. */
	virtual void ShutdownModule() override;

private:

	TSharedRef<SDockTab> SpawnTab(const FSpawnTabArgs& TabSpawnArgs);
};
