// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "LogicActors/LogicCondition.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicCondition::ALogicCondition()
{
	
}

void ALogicCondition::RegisterIOEvents(FActorIOEventList& EventRegistry)
{
	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCondition::OnPass"))
		.SetDisplayName(LOCTEXT("LogicCondition.OnPass", "OnPass"))
		.SetTooltipText(LOCTEXT("LogicCondition.OnPassTooltip", "Event when the condition passes after 'Test' is called."))
		.SetMulticastDelegate(this, &OnPass));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCondition::OnFail"))
		.SetDisplayName(LOCTEXT("LogicCondition.OnFail", "OnFail"))
		.SetTooltipText(LOCTEXT("LogicCondition.OnFailTooltip", "Event when the condition fails after 'Test' is called."))
		.SetMulticastDelegate(this, &OnFail));
}

void ALogicCondition::RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry)
{
	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicCondition::Test"))
		.SetDisplayName(LOCTEXT("LogicCondition.Test", "Test"))
		.SetTooltipText(LOCTEXT("LogicCondition.TestTooltip", "Test the condition and fire 'OnPass' or 'OnFail' based on the result."))
		.SetFunction(TEXT("Test")));
}

void ALogicCondition::Test()
{
	FString Result;
	bool bSuccess = Condition.Expression.Evaluate(Result);

	if (Result == TEXT("1") || Result == TEXT("True"))
	{
		OnPass.Broadcast();
	}
	else
	{
		OnFail.Broadcast();
	}
}

#undef LOCTEXT_NAMESPACE
