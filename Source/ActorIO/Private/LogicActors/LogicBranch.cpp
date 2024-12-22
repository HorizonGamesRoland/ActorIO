// Copyright 2024 Horizon Games. All Rights Reserved.

#include "LogicActors/LogicBranch.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicBranch::ALogicBranch()
{
	bInitialValue = false;
	bCurrentValue = false;
}

void ALogicBranch::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	bCurrentValue = bInitialValue;
}

void ALogicBranch::RegisterIOEvents_Implementation(TArray<FActorIOEvent>& RegisteredEvents)
{
	RegisteredEvents.Add(FActorIOEvent()
		.SetId(TEXT("ALogicBranch::OnTrue"))
		.SetDisplayName(LOCTEXT("LogicBranch.OnTrue", "OnTrue"))
		.SetTooltipText(LOCTEXT("LogicBranch.OnTrueTooltip", "Event when the stored boolean value is true when evaluating."))
		.SetMulticastDelegate(this, &TrueEvent));

	RegisteredEvents.Add(FActorIOEvent()
		.SetId(TEXT("ALogicBranch::OnFalse"))
		.SetDisplayName(LOCTEXT("LogicBranch.OnFalse", "OnFalse"))
		.SetTooltipText(LOCTEXT("LogicBranch.OnFalseTooltip", "Event when the stored boolean value is false when evaluating."))
		.SetMulticastDelegate(this, &FalseEvent));
}

void ALogicBranch::RegisterIOFunctions_Implementation(TArray<FActorIOFunction>& RegisteredFunctions)
{
	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicBranch::SetValue"))
		.SetDisplayName(LOCTEXT("LogicBranch.SetValue", "SetValue"))
		.SetTooltipText(LOCTEXT("LogicBranch.SetValueTooltip", "Set the boolean value without performing the comparison. Use this to hold a value for a future test."))
		.SetFunction(TEXT("SetValue")));

	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicBranch::SetValueAndEvaluate"))
		.SetDisplayName(LOCTEXT("LogicBranch.SetValueAndEvaluate", "SetValueAndEvaluate"))
		.SetTooltipText(LOCTEXT("LogicBranch.SetValueAndEvaluateTooltip", "Set the boolean value and test it, firing OnTrue or OnFalse based on the new value."))
		.SetFunction(TEXT("SetValueAndEvaluate")));

	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicBranch::Toggle"))
		.SetDisplayName(LOCTEXT("LogicBranch.Toggle", "Toggle"))
		.SetTooltipText(LOCTEXT("LogicBranch.ToggleTooltip", "Toggle the boolean value between true and false without performing the comparison."))
		.SetFunction(TEXT("Toggle")));

	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicBranch::ToggleAndEvaluate"))
		.SetDisplayName(LOCTEXT("LogicBranch.ToggleAndEvaluate", "ToggleAndEvaluate"))
		.SetTooltipText(LOCTEXT("LogicBranch.ToggleAndEvaluateTooltip", "Toggle the boolean value and tests it, firing OnTrue or OnFalse based on the new value."))
		.SetFunction(TEXT("ToggleAndEvaluate")));

	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicBranch::Evaluate"))
		.SetDisplayName(LOCTEXT("LogicBranch.Evaluate", "Evaluate"))
		.SetTooltipText(LOCTEXT("LogicBranch.EvaluateTooltip", "Test the input value and fire OnTrue or OnFalse based on the value."))
		.SetFunction(TEXT("Evaluate")));
}

void ALogicBranch::SetValue(bool bValue)
{
	bCurrentValue = bValue;
}

void ALogicBranch::SetValueAndEvaluate(bool bValue)
{
	bCurrentValue = bValue;
	Evaluate();
}

void ALogicBranch::Toggle()
{
	bCurrentValue = !bCurrentValue;
}

void ALogicBranch::ToggleAndEvaluate()
{
	bCurrentValue = !bCurrentValue;
	Evaluate();
}

void ALogicBranch::Evaluate()
{
	if (bCurrentValue)
	{
		TrueEvent.Broadcast();
	}
	else
	{
		FalseEvent.Broadcast();
	}
}

#undef LOCTEXT_NAMESPACE
