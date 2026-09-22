// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "IPropertyTypeCustomization.h"
#include "IDetailCustomNodeBuilder.h"
#include "ActorIOExpressions.h"

class IPropertyHandle;
class IPropertyHandleArray;
class IPropertyUtilities;
class IDetailLayoutBuilder;
class FReply;
class SWidget;

class ACTORIOEDITOR_API FActorIOExpressionDetailBuilder : public IDetailCustomNodeBuilder, public TSharedFromThis<FActorIOExpressionDetailBuilder>
{
public:

	FActorIOExpressionDetailBuilder();

	//~ Begin IDetailCustomNodeBuilder Interface
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRebuildChildren) override { OnRebuildChildren = InOnRebuildChildren; }
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
	//~ End IDetailCustomNodeBuilder Interface

	void SetExpression(FActorIOExpressionBase* InExpr) { Expr = InExpr; }

	template<class T>
	T* GetExpression() const { return static_cast<T*>(Expr); }

	void SetNameOverrideText(const FText& InText) { NameOverrideText = InText; }
	void SetAllowRemove(bool bEnabled) { bAllowRemove = bEnabled; }

protected:

	FActorIOExpressionBase* Expr;

	FText NameOverrideText;

	bool bAllowRemove;

	FSimpleDelegate OnRebuildChildren;

	IDetailLayoutBuilder* LayoutBuilder;
};

/**
 *
 */
class ACTORIOEDITOR_API FActorIOLiteralExpressionBuilder : public FActorIOExpressionDetailBuilder
{
public:

	/** Makes a new instance of this customization. */
	static TSharedRef<FActorIOExpressionDetailBuilder> MakeInstance();

	//~ Begin FActorIOExpressionDetailBuilder Interface
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual FName GetName() const override { return FName("ActorIOLiteralExpression"); }
	//~ End FActorIOExpressionDetailBuilder Interface

protected:

	TSharedPtr<class STextBlock> HeaderText;

protected:

	void UpdateHeaderText();

	FText OnGetValueText();

	void OnValueCommitted(const FText& InText, ETextCommit::Type InCommitType);

	void OnClick_Remove();
};

/**
 *
 */
class ACTORIOEDITOR_API FActorIOKismetFunctionExpressionBuilder : public FActorIOExpressionDetailBuilder
{
public:

	/** Makes a new instance of this customization. */
	static TSharedRef<FActorIOExpressionDetailBuilder> MakeInstance();

	//~ Begin FActorIOExpressionDetailBuilder Interface
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
	virtual FName GetName() const override { return FName("ActorIOKismetFunctionExpression"); }
	//~ End FActorIOExpressionDetailBuilder Interface

protected:

	TSharedPtr<class STextBlock> HeaderText;

	/**
	 * List of I/O function ids that are selectable in the function combo box.
	 * Always contains the same ids found in ValidFunctions above.
	 */
	TArray<FName> SelectableFunctions;

	// #todo: remove if OnReferencedFunctionChanged is not needed
	UFunction* ReferencedFunction;

protected:

	const UClass* OnGetFunctionClass() const;

	void OnSetFunctionClass(const UClass* SelectedClass);

	/** Called when generating an entry for the event combo box. */
	TSharedRef<SWidget> OnGenerateFunctionComboBoxWidget(FName InName);

	/** Called before the event combo box is opened. */
	void OnFunctionComboBoxOpening();

	/** Called when the selection changes for the event combo box. */
	void OnFunctionComboBoxSelectionChanged(FName InName, ESelectInfo::Type InSelectType);

	/** Finds the display name of the given I/O function. */
	FText GetFunctionDisplayName(FName InFunctionId) const;

	/** @return Color based on whether the given I/O function is valid or not. */
	FSlateColor GetFunctionDisplayColor(FName InFunctionId) const;

	/** @return Tooltip widget to use for I/O functions. */
	FText GetFunctionTooltip(FName InFunctionId);

	void OnReferencedFunctionChanged();

	void UpdateHeaderText();

	void UpdateSelectableFunctions();

	void OnClick_Negate();

	void OnClick_Remove();
};

/**
 *
 */
class ACTORIOEDITOR_API FActorIOGroupExpressionBuilder : public FActorIOExpressionDetailBuilder
{
public:

	/** Makes a new instance of this customization. */
	static TSharedRef<FActorIOExpressionDetailBuilder> MakeInstance();

	//~ Begin FActorIOExpressionDetailBuilder Interface
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
	virtual FName GetName() const override { return FName("ActorIOGroupExpression"); }
	//~ End FActorIOExpressionDetailBuilder Interface

protected:

	TSharedPtr<class STextBlock> HeaderText;

protected:

	void OnClick_AddCondition();

	void OnClick_Negate();

	void OnClick_Remove();

	void UpdateHeaderText();
};

class ACTORIOEDITOR_API FActorIOInvalidExpressionBuilder : public FActorIOExpressionDetailBuilder
{
public:

	/** Makes a new instance of this customization. */
	static TSharedRef<FActorIOExpressionDetailBuilder> MakeInstance();

	//~ Begin FActorIOExpressionDetailBuilder Interface
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual FName GetName() const override { return FName("ActorIOInvalidExpression"); }
	//~ End FActorIOExpressionDetailBuilder Interface
};

/**
 *
 */
class ACTORIOEDITOR_API FActorIOExpressionContainerCustomization : public IPropertyTypeCustomization
{
public:

	/** Makes a new instance of this customization. */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	//~ Begin IPropertyTypeCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	//~ End IPropertyTypeCustomization Interface

	FActorIOExpressionContainer* GetContainerData();

protected:

	TSharedPtr<IPropertyHandle> PropStruct;

	TSharedPtr<IPropertyHandleArray> PropExpressions;

	TSharedPtr<IPropertyUtilities> PropUtilities;

protected:

	FText GetHeaderText();

	void OnClick_AddExpression();

	void OnClick_ResetExpressions();

	void OnClick_DebugExpressions();

	void OnValueChanged();
};

class ACTORIOEDITOR_API FActorIODetailCustomizationHelper
{
public:

	// #todo: create registry similar to PropertyModule.RegisterCustomClassLayout

	static TSharedRef<FActorIOExpressionDetailBuilder> NewExpressionDetailBuilderOfType(FName TypeName);
};
