// Copyright 2025 Horizon Games. All Rights Reserved.

#include "LogicActors/LogicCondition.h"

#define LOCTEXT_NAMESPACE "ActorIO"

ALogicCondition::ALogicCondition()
{
	ObjectToTest = nullptr;
	FunctionName = NAME_None;

#if WITH_EDITORONLY_DATA
	ConstructorHelpers::FObjectFinderOptional<UTexture2D> SpriteTexture(TEXT("/ActorIO/S_Condition"));
	if (SpriteComponent && SpriteTexture.Succeeded())
	{
		SpriteComponent->SetSprite(SpriteTexture.Get());
		SpriteComponent->SetRelativeScale3D_Direct(FVector(1.0f));
	}
#endif
}

void ALogicCondition::RegisterIOEvents(FActorIOEventList& EventRegistry)
{
	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCondition::OnPass"))
		.SetDisplayName(LOCTEXT("ALogicCondition.OnPass", "OnPass"))
		.SetTooltipText(LOCTEXT("ALogicCondition.OnPassTooltip", "Event when the condition function returns true."))
		.SetMulticastDelegate(this, &OnPass));

	EventRegistry.RegisterEvent(FActorIOEvent()
		.SetId(TEXT("ALogicCondition::OnFail"))
		.SetDisplayName(LOCTEXT("ALogicCondition.OnFail", "OnFail"))
		.SetTooltipText(LOCTEXT("ALogicCondition.OnFailTooltip", "Event when the condition function returns false."))
		.SetMulticastDelegate(this, &OnFail));
}

void ALogicCondition::RegisterIOFunctions(FActorIOFunctionList& FunctionRegistry)
{
	FunctionRegistry.RegisterFunction(FActorIOFunction()
		.SetId(TEXT("ALogicCondition::Test"))
		.SetDisplayName(LOCTEXT("ALogicCondition.Test", "Test"))
		.SetTooltipText(LOCTEXT("ALogicCondition.TestTooltip", "Test the return value of the condition function and fire 'OnPass' or 'OnFail' based on the value."))
		.SetFunction(TEXT("Test")));
}

void ALogicCondition::Test(const FString& Arguments) const
{
	if (!IsValid(ObjectToTest))
	{
		OnFail.Broadcast();
		return;
	}

	const bool bResult = PerformConditionCheck(FunctionName, Arguments);
	if (bResult)
	{
		OnPass.Broadcast();
	}
	else
	{
		OnFail.Broadcast();
	}
}

bool ALogicCondition::PerformConditionCheck(FName InFunctionName, const FString& Arguments) const
{
	UFunction* Function = ObjectToTest->GetClass()->FindFunctionByName(InFunctionName);
	if (!Function)
	{
		return false;
	}

	FBoolProperty* ReturnProp = CastField<FBoolProperty>(Function->GetReturnProperty());
	if (!ReturnProp)
	{
		return false;
	}

	FStructOnScope FuncParam(Function);

	// Set input properties
	//FNameProperty* InProp = CastField<FNameProperty>(Function->FindPropertyByName(TEXT("CurveName")));
	//if (!InProp)
	//{
	//	return NAME_None;
	//}
	//InProp->SetPropertyValue_InContainer(FuncParam.GetStructMemory(), InCurveName);

	// Call function
	ObjectToTest->ProcessEvent(Function, FuncParam.GetStructMemory());

	const bool bResult = ReturnProp->GetPropertyValue_InContainer(FuncParam.GetStructMemory());

	//!!destructframe see also UObject::ProcessEvent
	for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		It->DestroyValue_InContainer(FuncParam.GetStructMemory());
	}

	return bResult;
}

#undef LOCTEXT_NAMESPACE