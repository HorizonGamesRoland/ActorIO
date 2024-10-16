// Copyright 2024 Horizon Games. All Rights Reserved.

#pragma once

#include "ActorIO.h"
#include "Modules/ModuleManager.h"

class FActorIOModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
};
