// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "LogicActors/LogicScriptCondition.h"
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicScriptCondition::ALogicScriptCondition()
{
	Conditions.SetIsConditionContainer(true);
	Conditions.InitFromString(TEXT("and()"));
}

void ALogicScriptCondition::RegisterIOEvents(FActorIOEventList& EventRegistry)
{
	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicScriptCondition::OnPass"))
		.SetDisplayName(LOCTEXT("LogicCondition.OnPass", "OnPass"))
		.SetTooltipText(LOCTEXT("LogicCondition.OnPassTooltip", "Event when the condition passes after 'Test' is called."))
		.SetMulticastDelegate(this, &OnPass));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicScriptCondition::OnFail"))
		.SetDisplayName(LOCTEXT("LogicCondition.OnFail", "OnFail"))
		.SetTooltipText(LOCTEXT("LogicCondition.OnFailTooltip", "Event when the condition fails after 'Test' is called."))
		.SetMulticastDelegate(this, &OnFail));
}

void ALogicScriptCondition::RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry)
{
	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicScriptCondition::Test"))
		.SetDisplayName(LOCTEXT("LogicCondition.Test", "Test"))
		.SetTooltipText(LOCTEXT("LogicCondition.TestTooltip", "Test the condition and fire 'OnPass' or 'OnFail' based on the result."))
		.SetFunction(TEXT("Test")));
}

#if WITH_EDITOR
void ALogicScriptCondition::CheckForErrors()
{
	Super::CheckForErrors();

	if (Conditions.HasAnyErrors())
	{
		FMessageLog("MapCheck").Error()
			->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOPrefix", "[I/O]")))
			->AddToken(FUObjectToken::Create(this))
			->AddToken(FTextToken::Create(LOCTEXT("MapCheck_Message_IOExpressionContainerError", "has an expression container with error(s).")));
	}
}
#endif

void ALogicScriptCondition::Test()
{
	if (Conditions.Evaluate(this))
	{
		OnPass.Broadcast();
	}
	else
	{
		OnFail.Broadcast();
	}
}

#undef LOCTEXT_NAMESPACE
