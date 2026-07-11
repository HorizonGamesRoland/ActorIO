// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIODetailsCustomization.h"
#include "ActorIOExpression.h"
#include "IDetailChildrenBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"

TSharedRef<IPropertyTypeCustomization> FActorIOFunctionExpressionCustomization::MakeInstance()
{
	return MakeShareable(new FActorIOFunctionExpressionCustomization);
}

void FActorIOFunctionExpressionCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	//HeaderRow.NameContent()
	//[
	//	StructPropertyHandle->CreatePropertyNameWidget()
	//];

	TSharedPtr<IPropertyHandle> PropClass = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FActorIOFunctionExpression, ClassRef));
	TSharedPtr<IPropertyHandle> PropFunctionId = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FActorIOFunctionExpression, FunctionId));

	HeaderRow.NameContent()
	[
		PropClass->CreatePropertyValueWidget()
	]
	.ValueContent()
	[
		PropFunctionId->CreatePropertyValueWidget()
	];
}

void FActorIOFunctionExpressionCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	//TSharedPtr<IPropertyHandle> PropClass = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FActorIOFunctionExpression, ClassRef));
	//StructBuilder.AddProperty(PropClass.ToSharedRef());
	//
	//TSharedPtr<IPropertyHandle> PropFunctionId = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FActorIOFunctionExpression, FunctionId));
	//StructBuilder.AddProperty(PropFunctionId.ToSharedRef());

	TSharedPtr<IPropertyHandleArray> PropArgs = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FActorIOFunctionExpression, Args))->AsArray();

	uint32 NumArgs = 0;
	PropArgs->GetNumElements(NumArgs);

	for (uint32 ArgIdx = 0; ArgIdx != NumArgs; ++ArgIdx)
	{
		TSharedRef<IPropertyHandle> PropArg = PropArgs->GetElement(ArgIdx);
		StructBuilder.AddProperty(PropArg);
	}



	TSharedPtr<IPropertyHandle> PropArgs2 = StructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FActorIOFunctionExpression, Args));
	StructBuilder.AddProperty(PropArgs2.ToSharedRef());

	//TArray<TInstancedStruct<FActorIOExpressionBase>>& ArgsArray = PropArgs->getvalue
}

