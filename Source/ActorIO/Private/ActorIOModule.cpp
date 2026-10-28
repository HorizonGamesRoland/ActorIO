// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIOModule.h"
#include "ActorIOExpressions.h"

IMPLEMENT_MODULE(FActorIOModule, ActorIO)

void FActorIOModule::StartupModule()
{
	ExpressionRegistry = MakeUnique<FActorIOExpressionRegistry>();
	ExpressionRegistry->Initialize();
}

void FActorIOModule::ShutdownModule()
{
	ExpressionRegistry->Shutdown();
	ExpressionRegistry.Reset();
}
