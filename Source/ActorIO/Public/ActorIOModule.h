// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "Modules/ModuleManager.h"

class FActorIOExpressionRegistry;

/**
 * Runtime module implementation of the Actor I/O plugin.
 */
class FActorIOModule : public IModuleInterface
{
private:

	/** The global expression registry instance. */
	TUniquePtr<FActorIOExpressionRegistry> ExpressionRegistry;

public:

	/** @return The global expression registry instance. */
	FActorIOExpressionRegistry* GetExpressionRegistry() const { return ExpressionRegistry.Get(); }

public:

	//~ Begin IModuleInterface Interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface Interface
};
