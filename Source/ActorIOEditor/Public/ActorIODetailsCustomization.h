// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "IPropertyTypeCustomization.h"
#include "IDetailCustomNodeBuilder.h"
#include "ActorIOExpressions.h"

class IPropertyHandle;
class IPropertyHandleArray;
class IPropertyUtilities;
class IDetailLayoutBuilder;
class SWidget;
class FReply;

/**
 *
 */
class ACTORIOEDITOR_API FActorIOExpressionContainerCustomization : public IPropertyTypeCustomization
{
public:

	FActorIOExpressionContainerCustomization();
	~FActorIOExpressionContainerCustomization();

	/** Makes a new instance of this customization. */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	//~ Begin IPropertyTypeCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	//~ End IPropertyTypeCustomization Interface

	FActorIOExpressionContainer* GetContainerData() const;

	TSharedPtr<IPropertyHandle> GetExpressionProperty(int32 Idx) const;

protected:

	TSharedPtr<IPropertyHandle> PropStruct;

	TSharedPtr<IPropertyHandle> PropExpressionArray;

	TSharedPtr<IPropertyUtilities> PropUtilities;

	bool bRootExpressionHidden;

	FDelegateHandle DelegateHandle_FixupContainerReferences;

	FDelegateHandle DelegateHandle_BlueprintCompiled;

protected:

	FText GetHeaderText() const;

	FText GetHeaderTooltip() const;

	void OnContainerDataChanged();

	void OnFixupContainerReferences();

	void OnExpressionArrayChanged();

	void OnBlueprintCompiled();

	TSharedRef<SWidget> GenerateAddExpressionMenu();

	void OnClick_AddExpression(FString InType);

	void OnClick_ClearExpressions();

	void OnClick_DebugExpressions();
};

class ACTORIOEDITOR_API FActorIOExpressionDetailBuilder : public IDetailCustomNodeBuilder, public TSharedFromThis<FActorIOExpressionDetailBuilder>
{
public:

	FActorIOExpressionDetailBuilder();

	//~ Begin IDetailCustomNodeBuilder Interface
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRebuildChildren) override { OnRebuildChildren = InOnRebuildChildren; }
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual bool InitiallyCollapsed() const override { return false; }
	virtual FName GetName() const override { return NAME_None; }
	//~ End IDetailCustomNodeBuilder Interface

	virtual bool WholeRowHeader() const { return false; }

	virtual void RefreshHeader();

	void SetExpression(const TSharedPtr<IPropertyHandle>& InProperty, FActorIOExpressionBase* InExpr);

	template<class T>
	T* GetExpression() const { return static_cast<T*>(Expr); }

	void SetParentCustomization(FActorIOExpressionContainerCustomization* InCustomization) { ParentCustomization = InCustomization; }

	void SetHeaderTextOverride(const FText& InText) { HeaderTextOverride = InText; }

	void SetAllowRemove(bool bEnabled) { bAllowRemove = bEnabled; }

protected:

	TSharedPtr<class STextBlock> HeaderText;

	TSharedPtr<class SHorizontalBox> ExtensionBox;

	TSharedPtr<IPropertyHandle> PropExpression;

	TSharedPtr<IPropertyHandle> PropExpressionArray;

	FActorIOExpressionBase* Expr;

	FActorIOExpressionContainerCustomization* ParentCustomization;

	FText HeaderTextOverride;

	bool bAllowRemove;

	FSimpleDelegate OnRebuildChildren;

protected:

	void OnClick_Remove();
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
	virtual void RefreshHeader() override;
	//~ End FActorIOExpressionDetailBuilder Interface

protected:

	FText OnGetValueText() const;

	void OnValueCommitted(const FText& InText, ETextCommit::Type InCommitType);
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
	virtual bool WholeRowHeader() const { return true; }
	virtual void RefreshHeader() override;
	//~ End FActorIOExpressionDetailBuilder Interface

protected:

	/**
	 * List of I/O function ids that are selectable in the function combo box.
	 * Always contains the same ids found in ValidFunctions above.
	 */
	TArray<FName> SelectableFunctions;

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
	FText GetFunctionDisplayName(FName InFunctionName) const;

	/** @return Color based on whether the given I/O function is valid or not. */
	FSlateColor GetFunctionDisplayColor(FName InFunctionName) const;

	/** @return Tooltip widget to use for I/O functions. */
	FText GetFunctionTooltip(FName InFunctionName);

	void UpdateSelectableFunctions();

	void OnClick_Negate();

	FReply OnClick_FixArguments();
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
	virtual bool WholeRowHeader() const { return true; }
	virtual void RefreshHeader() override;
	//~ End FActorIOExpressionDetailBuilder Interface

protected:

	TSharedRef<SWidget> GenerateAddExpressionMenu();

	void OnClick_AddExpression(FString InType);

	void OnClick_Negate();
};

class ACTORIOEDITOR_API FActorIOInvalidExpressionBuilder : public FActorIOExpressionDetailBuilder
{
public:

	/** Makes a new instance of this customization. */
	static TSharedRef<FActorIOExpressionDetailBuilder> MakeInstance();

	//~ Begin FActorIOExpressionDetailBuilder Interface
	virtual void RefreshHeader() override;
	//~ End FActorIOExpressionDetailBuilder Interface
};

class ACTORIOEDITOR_API FActorIODetailCustomizationHelper
{
public:

	// #todo: create registry similar to PropertyModule.RegisterCustomClassLayout

	static TSharedRef<FActorIOExpressionDetailBuilder> NewExpressionDetailBuilderOfType(FName TypeName);
};
