// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "LogicActors/LogicCondition.h"
#include "ActorIOSubsystemBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"
#include "Misc/OutputDeviceNull.h"

#define LOCTEXT_NAMESPACE "ActorIO"

bool UActorIOExpression::Evaluate(FString& OutResult)
{
	OutResult.Empty();

	FString Cmd = FunctionId.ToString();
	for (UActorIOExpression* Expr : Args)
	{
		FString Result;
		if (!Expr->Evaluate(Result))
		{
			return false;
		}

		Cmd += TEXT(" ");
		Cmd += Result;
	}

	UActorIOSubsystemBase* IOSubsystem = UActorIOSubsystemBase::Get(GEngine->GetCurrentPlayWorld());
	check(IOSubsystem);

	UObject* Target = ObjectPtr.Get();
	if (bFunctionIsKismetOp)
	{
		Target = GetMutableDefault<UKismetMathLibrary>();
	}

	FOutputDeviceNull Ar;
	return IOSubsystem->ExecuteCommand(Target, *Cmd, Ar, IOSubsystem, &OutResult);
}

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
	if (!Expression)
	{
		OnPass.Broadcast();
		return;
	}

	FString Result;
	bool bSuccess = Expression->Evaluate(Result);

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
