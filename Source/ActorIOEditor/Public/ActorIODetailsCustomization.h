// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "IPropertyTypeCustomization.h"
#include "IDetailCustomNodeBuilder.h"
#include "ActorIOExpression.h"

class IPropertyHandle;
class IPropertyUtilities;
class FReply;

struct ACTORIOEDITOR_API FActorIOExpressionBuilderParams
{
	FActorIOExpressionBase* Expr;

	FText HeaderNameOverrideText;

	bool bCanRemoveExpression;

	FActorIOExpressionBuilderParams() :
		Expr(nullptr),
		HeaderNameOverrideText(FText::GetEmpty()),
		bCanRemoveExpression(true)
	{}
};

class ACTORIOEDITOR_API FActorIODetailCustomizationHelper
{
public:

	static TSharedRef<IDetailCustomNodeBuilder> GenerateExpressionDetailRow(const FActorIOExpressionBuilderParams& InParams);
};

/**
 * 
 */
class ACTORIOEDITOR_API FActorIOScriptConditionCustomization : public IPropertyTypeCustomization
{
public:

	/** Makes a new instance of this customization. */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	//~ Begin IPropertyTypeCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	//~ End IPropertyTypeCustomization Interface

protected:

	FActorIOScriptCondition* Struct;

	TSharedPtr<IPropertyUtilities> PropUtilities;

protected:

	void OnClick_AddCondition();

	void OnClick_ResetConditions();
};

/**
 *
 */
class ACTORIOEDITOR_API FActorIOLiteralExpressionBuilder : public IDetailCustomNodeBuilder, public TSharedFromThis<FActorIOLiteralExpressionBuilder>
{
public:

	FActorIOLiteralExpressionBuilder(const FActorIOExpressionBuilderParams& InParams);

	//~ Begin IDetailCustomNodeBuilder Interface
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual FName GetName() const override { return FName(TEXT("ActorIOLiteralExpression")); }
	//~ End IDetailCustomNodeBuilder Interface

protected:

	FActorIOExpressionBuilderParams Params;
	FActorIOLiteralExpression* Expr;

	TSharedPtr<class STextBlock> HeaderText;

protected:

	void UpdateHeaderText();

	void OnClick_Remove();
};

/**
 *
 */
class ACTORIOEDITOR_API FActorIOKismetFunctionExpressionBuilder : public IDetailCustomNodeBuilder, public TSharedFromThis<FActorIOKismetFunctionExpressionBuilder>
{
public:

	FActorIOKismetFunctionExpressionBuilder(const FActorIOExpressionBuilderParams& InParams);

	//~ Begin IDetailCustomNodeBuilder Interface
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRebuildChildren) override { OnRebuildChildren = InOnRebuildChildren; }
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
	virtual FName GetName() const override { return FName("ActorIOKismetFunctionExpression"); }
	//~ End IDetailCustomNodeBuilder Interface

protected:

	FActorIOExpressionBuilderParams Params;
	FActorIOKismetFunctionExpression* Expr;

	// #todo: remove if OnReferencedFunctionChanged is not needed
	UFunction* ReferencedFunction;

	FSimpleDelegate OnRebuildChildren;

	TSharedPtr<class STextBlock> HeaderText;

protected:

	const UClass* OnGetFunctionClass() const;

	void OnSetFunctionClass(const UClass* SelectedClass);

	FName OnGetFunctionId();

	void OnSetFunctionId(const FText& NewText, ETextCommit::Type CommitInfo);

	void OnReferencedFunctionChanged();

	void UpdateHeaderText();

	void OnClick_Remove();
};

/**
 *
 */
class ACTORIOEDITOR_API FActorIOGroupExpressionBuilder : public IDetailCustomNodeBuilder, public TSharedFromThis<FActorIOGroupExpressionBuilder>
{
public:

	FActorIOGroupExpressionBuilder(const FActorIOExpressionBuilderParams& InParams);

	//~ Begin IDetailCustomNodeBuilder Interface
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRebuildChildren) override { OnRebuildChildren = InOnRebuildChildren; }
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
	virtual FName GetName() const override { return FName("ActorIOGroupExpression"); }
	//~ End IDetailCustomNodeBuilder Interface

protected:
	
	FActorIOExpressionBuilderParams Params;
	FActorIOGroupExpression* Expr;

	FSimpleDelegate OnRebuildChildren;

protected:

	FReply OnAddFunctionClicked();

	FReply OnAddGroupClicked();

	void OnClick_Remove();
};

class ACTORIOEDITOR_API FActorIOInvalidExpressionBuilder : public IDetailCustomNodeBuilder, public TSharedFromThis<FActorIOInvalidExpressionBuilder>
{
public:

	FActorIOInvalidExpressionBuilder(const FActorIOExpressionBuilderParams& InParams);

	//~ Begin IDetailCustomNodeBuilder Interface
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual FName GetName() const override { return FName("ActorIOInvalidExpression"); }
	//~ End IDetailCustomNodeBuilder Interface

protected:

	FActorIOExpressionBuilderParams Params;
};
