// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "LogicActors/LogicScriptAction.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicScriptAction::ALogicScriptAction()
{
	Actions.SetIsConditionContainer(false);
}

void ALogicScriptAction::RegisterIOEvents(FActorIOEventList& EventRegistry)
{
	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicScriptAction::OnExecute"))
		.SetDisplayName(LOCTEXT("LogicScriptAction.OnExecute", "OnExecute"))
		.SetTooltipText(LOCTEXT("LogicScriptAction.OnExecuteTooltip", "Event when the actions are executed."))
		.SetMulticastDelegate(this, &OnExecute));
}

void ALogicScriptAction::RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry)
{
	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicScriptAction::Execute"))
		.SetDisplayName(LOCTEXT("LogicScriptAction.Execute", "Execute"))
		.SetTooltipText(LOCTEXT("LogicScriptAction.ExecuteTooltip", "Execute the actions and fire the 'OnExecute' event."))
		.SetFunction(TEXT("Execute")));
}

bool ALogicScriptAction::Execute()
{
	bool bResult = Actions.Evaluate(this);
	OnExecute.Broadcast();

	return bResult;
}

#undef LOCTEXT_NAMESPACE
