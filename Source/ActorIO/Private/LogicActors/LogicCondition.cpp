// Copyright 2024 Horizon Games. All Rights Reserved.

#include "LogicActors/LogicCondition.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicCondition::ALogicCondition()
{
	ObjectToTest = nullptr;
	FunctionName = FString();
}

void ALogicCondition::RegisterIOEvents_Implementation(TArray<FActorIOEvent>& RegisteredEvents)
{
	RegisteredEvents.Add(FActorIOEvent()
		.SetId(TEXT("ALogicCondition::OnPass"))
		.SetDisplayName(LOCTEXT("ALogicCondition.OnPass", "OnPass"))
		.SetTooltipText(LOCTEXT("ALogicCondition.OnPassTooltip", "Event when the condition function returns true."))
		.SetMulticastDelegate(this, &OnPass));

	RegisteredEvents.Add(FActorIOEvent()
		.SetId(TEXT("ALogicCondition::OnFail"))
		.SetDisplayName(LOCTEXT("ALogicCondition.OnFail", "OnFail"))
		.SetTooltipText(LOCTEXT("ALogicCondition.OnFailTooltip", "Event when the condition function returns false."))
		.SetMulticastDelegate(this, &OnFail));
}

void ALogicCondition::RegisterIOFunctions_Implementation(TArray<FActorIOFunction>& RegisteredFunctions)
{
	RegisteredFunctions.Add(FActorIOFunction()
		.SetId(TEXT("ALogicCondition::Test"))
		.SetDisplayName(LOCTEXT("ALogicCondition.Test", "Test"))
		.SetTooltipText(LOCTEXT("ALogicCondition.TestTooltip", "Test the return value of the condition function and fire 'OnPass' or 'OnFail' based on the value."))
		.SetFunction(TEXT("Test")));
}

void ALogicCondition::Test(const FString& Arguments) const
{
	// #TODO: Figure out how to get return value.

	//UFunction* Func_GetRemappedCurveName = RemapAsset->GetClass()->FindFunctionByName(FName("GetRemappedCurveName"));
	//if (!Func_GetRemappedCurveName)
	//{
	//	return NAME_None;
	//}
	//
	//FStructOnScope FuncParam(Func_GetRemappedCurveName);
	//
	//// Set input properties
	//FNameProperty* InProp = CastField<FNameProperty>(Func_GetRemappedCurveName->FindPropertyByName(TEXT("CurveName")));
	//if (!InProp)
	//{
	//	return NAME_None;
	//}
	//InProp->SetPropertyValue_InContainer(FuncParam.GetStructMemory(), InCurveName);
	//
	//// Call function
	//RemapAsset->ProcessEvent(Func_GetRemappedCurveName, FuncParam.GetStructMemory());
	//
	//// Get return property
	//FNameProperty* OutProp = CastField<FNameProperty>(Func_GetRemappedCurveName->GetReturnProperty());
	//if (!OutProp)
	//{
	//	return NAME_None;
	//}
	//
	//return OutProp->GetPropertyValue_InContainer(FuncParam.GetStructMemory());
}

#undef LOCTEXT_NAMESPACE