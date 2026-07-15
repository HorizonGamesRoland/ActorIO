// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIODetailsCustomization.h"
#include "ActorIOExpression.h"
#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"

#define LOCTEXT_NAMESPACE "ActorIOEditor"

TSharedRef<IPropertyTypeCustomization> FActorIOFunctionExpressionCustomization::MakeInstance()
{
	return MakeShareable(new FActorIOFunctionExpressionCustomization);
}

void FActorIOFunctionExpressionCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	PropClass = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FActorIOFunctionExpression, ClassRef));
	PropFunctionId = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FActorIOFunctionExpression, FunctionId));
	PropArgs = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FActorIOFunctionExpression, Args))->AsArray();
	PropUtilities = StructCustomizationUtils.GetPropertyUtilities();

	FSimpleDelegate FunctionChangedDelegate = FSimpleDelegate::CreateSP(this, &FActorIOFunctionExpressionCustomization::OnFunctionReferenceChanged);
	PropClass->SetOnPropertyValueChanged(FunctionChangedDelegate);
	PropClass->SetOnPropertyResetToDefault(FunctionChangedDelegate);
	PropFunctionId->SetOnPropertyValueChanged(FunctionChangedDelegate);
	PropFunctionId->SetOnPropertyResetToDefault(FunctionChangedDelegate);

	UFunction* FunctionPtr = GetReferencedFunction();
	TArray<FProperty*> FunctionParams = GetFunctionParams(FunctionPtr);

	// Ensure argument count matches the function before drawing child properties.
	uint32 NumArgs = 0;
	if (PropArgs->GetNumElements(NumArgs) == FPropertyAccess::Success)
	{
		if (NumArgs != (uint32)FunctionParams.Num())
		{
			SetArgsCount(FunctionParams.Num());
		}
	}

	FText NameOverride = LOCTEXT("ExpressionEd_SelectFunction", "Select a function...");
	if (FunctionPtr)
	{
		NameOverride = FunctionPtr->GetDisplayNameText();
	}

	HeaderRow
	.WholeRowContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget(NameOverride)
	];
}

void FActorIOFunctionExpressionCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	UFunction* FunctionPtr = GetReferencedFunction();
	if (!FunctionPtr)
	{
		StructBuilder.AddProperty(PropClass.ToSharedRef());
		StructBuilder.AddProperty(PropFunctionId.ToSharedRef());
		return;
	}

	TArray<FProperty*> FunctionParams = GetFunctionParams(FunctionPtr);

	uint32 NumArgs = 0;
	PropArgs->GetNumElements(NumArgs);
	if (NumArgs != (uint32)FunctionParams.Num())
	{
		ensureAlwaysMsgf(false, TEXT("Invalid argument count for the referenced function!"));
		return;
	}

	for (int32 ArgIdx = 0; ArgIdx != FunctionParams.Num(); ++ArgIdx)
	{
		TSharedRef<IPropertyHandle> PropArg = PropArgs->GetElement(ArgIdx);
		IDetailPropertyRow& Row = StructBuilder.AddProperty(PropArg);

		FString ParamName = FunctionParams[ArgIdx]->GetAuthoredName();
		Row.DisplayName(FText::FromString(ParamName));

		FText ParamTooltip = FunctionParams[ArgIdx]->GetToolTipText();
		Row.ToolTip(ParamTooltip);
	}

	//TSharedPtr<IPropertyHandle> PropArgs2 = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FActorIOFunctionExpression, Args));
	//StructBuilder.AddProperty(PropArgs2.ToSharedRef());
}

void FActorIOFunctionExpressionCustomization::OnFunctionReferenceChanged()
{
	UFunction* FunctionPtr = GetReferencedFunction();
	TArray<FProperty*> FunctionParams = GetFunctionParams(FunctionPtr);
	SetArgsCount(FunctionParams.Num());

	if (PropUtilities.IsValid())
	{
		PropUtilities->RequestForceRefresh();
	}
}

void FActorIOFunctionExpressionCustomization::SetArgsCount(int32 InCount)
{
	if (PropArgs.IsValid())
	{
		PropArgs->EmptyArray();
		for (int32 ArgIdx = 0; ArgIdx != InCount; ++ArgIdx)
		{
			PropArgs->AddItem();
		}
	}
}

UFunction* FActorIOFunctionExpressionCustomization::GetReferencedFunction()
{
	UClass* ClassPtr = nullptr;
	UObject* ClassObject = nullptr;
	if (PropClass.IsValid() && PropClass->GetValue(ClassObject) == FPropertyAccess::Success)
	{
		ClassPtr = Cast<UClass>(ClassObject);
	}

	FName FuncName = NAME_None;
	if (PropFunctionId.IsValid() && PropFunctionId->GetValue(FuncName) == FPropertyAccess::Success)
	{
		if (ClassPtr && FuncName != NAME_None)
		{
			return ClassPtr->FindFunctionByName(FuncName);
		}
	}

	return nullptr;
}

TArray<FProperty*> FActorIOFunctionExpressionCustomization::GetFunctionParams(UFunction* InFunctionPtr)
{
	TArray<FProperty*> OutParams;

	if (InFunctionPtr)
	{
		for (TFieldIterator<FProperty> It(InFunctionPtr); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
		{
			FProperty* FunctionProp = *It;
			checkSlow(FunctionProp);

			// Do not create widget for return property.
			if (FunctionProp->HasAnyPropertyFlags(CPF_ReturnParm))
			{
				continue;
			}

			// Do not create widget for output params, but only if they are not passed by ref
			// since in that case the value is also an input param.
			if (FunctionProp->HasAnyPropertyFlags(CPF_OutParm) && !FunctionProp->HasAnyPropertyFlags(CPF_ReferenceParm))
			{
				continue;
			}

			// Skip blueprint generated '__WorldContext' property.
			// The value for this will be auto initialized for us by the 'ActorIO::ExecuteCommand' function.
			if (FunctionProp->GetName() == TEXT("__WorldContext"))
			{
				continue;
			}

			OutParams.Add(FunctionProp);
		}
	}

	return OutParams;
}

#undef LOCTEXT_NAMESPACE
