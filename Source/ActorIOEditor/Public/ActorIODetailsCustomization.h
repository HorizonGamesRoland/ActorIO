// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "IPropertyTypeCustomization.h"

class IPropertyHandle;
class IPropertyHandleArray;
class IPropertyUtilities;

/**
 * 
 */
class ACTORIOEDITOR_API FActorIOFunctionExpressionCustomization : public IPropertyTypeCustomization
{
public:

	/** Makes a new instance of this customization. */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	//~ Begin IPropertyTypeCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	//~ End IPropertyTypeCustomization Interface

protected:

	TSharedPtr<IPropertyHandle> PropClass;

	TSharedPtr<IPropertyHandle> PropFunctionId;

	TSharedPtr<IPropertyHandleArray> PropArgs;

	TSharedPtr<IPropertyUtilities> PropUtilities;

protected:

	void OnFunctionReferenceChanged();

	void SetArgsCount(int32 InCount);

	UFunction* GetReferencedFunction();

	TArray<FProperty*> GetFunctionParams(UFunction* InFunctionPtr);
};
