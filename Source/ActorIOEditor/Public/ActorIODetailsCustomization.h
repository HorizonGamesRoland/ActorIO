// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#pragma once

#include "IPropertyTypeCustomization.h"
#include "IDetailCustomNodeBuilder.h"
#include "IDetailDragDropHandler.h"
#include "ActorIOExpressions.h"

class IPropertyHandle;
class IPropertyHandleArray;
class IPropertyUtilities;
class IDetailLayoutBuilder;
class SWidget;
class FReply;

/**
 * Property customization for ActorIOExpressionContainer.
 */
class ACTORIOEDITOR_API FActorIOExpressionContainerCustomization : public IPropertyTypeCustomization
{
public:

	/** Default constructor / destructor. */
	FActorIOExpressionContainerCustomization();
	~FActorIOExpressionContainerCustomization();

	/** Makes a new instance of this customization. */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	//~ Begin IPropertyTypeCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	//~ End IPropertyTypeCustomization Interface

	/** Requests a rebuild of the details panel. */
	void RefreshDetails();

	/** @return Mutable struct data of the edited ActorIOExpressionContainer property. */
	FActorIOExpressionContainer* GetContainerData() const;

	/**
	 * Gets a reflected property element from the 'Expressions' array of the container.
	 * Result is TInstancedStruct<FActorIOExpressionBase> property.
	 */
	TSharedPtr<IPropertyHandle> GetExpressionProperty(int32 Idx) const;

	/**
	 * Gets the reflected 'Expressions' array of the container.
	 * Result is TArray<TInstancedStruct<FActorIOExpressionBase>> property.
	 */
	TSharedPtr<IPropertyHandle> GetExpressionArrayProperty() const { return PropExpressionArray; }

protected:

	/** Property reference of the ActorIOExpressionContainer itself. */
	TSharedPtr<IPropertyHandle> PropStruct;

	/** Property reference of the 'Expressions' array of the ActorIOExpressionContainer. */
	TSharedPtr<IPropertyHandle> PropExpressionArray;

	/** Cached reference to property utilities. */
	TSharedPtr<IPropertyUtilities> PropUtilities;

	/** Whether the 'HideRootExpression' meta tag is applied to the property. */
	bool bRootExpressionHidden;

	/** Handle for when container references are fixed up for elements in the container. */
	FDelegateHandle DelegateHandle_FixupContainerReferences;

	/** Handle for when a blueprint is compiled in the editor. */
	FDelegateHandle DelegateHandle_BlueprintCompiled;

protected:

	/** @return Text to display in the header row. */
	FText GetHeaderText() const;

	/** @return Text to display as tooltip of the header text. */
	FText GetHeaderTooltip() const;

	/** @return Widget to display in the context menu when adding new expressions. */
	TSharedRef<SWidget> GenerateAddExpressionMenu();

	/** Called when an expression is selected from the context menu. */
	void OnClick_AddExpression(FString InType);

	/** Called when the clear array button is pressed. */
	void OnClick_ClearExpressions();

	/** Called when the debug button is pressed. */
	void OnClick_DebugExpressions();
};

/**
 * Base class for customizing expression rows within FActorIOExpressionContainerCustomization.
 */
class ACTORIOEDITOR_API FActorIOExpressionDetailBuilder : public IDetailCustomNodeBuilder, public TSharedFromThis<FActorIOExpressionDetailBuilder>
{
public:

	/** Default constructor. */
	FActorIOExpressionDetailBuilder();

	//~ Begin IDetailCustomNodeBuilder Interface
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRebuildChildren) override { OnRebuildChildren = InOnRebuildChildren; }
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual bool InitiallyCollapsed() const override { return false; }
	virtual FName GetName() const override { return NAME_None; }
	//~ End IDetailCustomNodeBuilder Interface

	/** Whether the header row should use the full length layout, or separated header / value columns. */
	virtual bool WholeRowHeader() const { return false; }

	/** Refreshes the header row of the expression. */
	virtual void RefreshHeader();

	/** Refreshes the error display of the expression. */
	virtual void RefreshError();

	/** Set the expression that this row represents. */
	void SetExpression(const TSharedPtr<IPropertyHandle>& InProperty, TInstancedStruct<FActorIOExpressionBase>* InExpr);

	/** Set the owning container customization. */
	void SetParentCustomization(FActorIOExpressionContainerCustomization* InCustomization) { ParentCustomization = InCustomization; }

	/** Set custom header text for this expression. */
	void SetHeaderTextOverride(const FText& InText) { HeaderTextOverride = InText; }

	/** Set whether this expression can be removed. */
	void SetAllowRemove(bool bEnabled) { bAllowRemove = bEnabled; }

	/** Set whether this expression can be drag-dropped. */
	void SetAllowDragDrop(bool bEnabled) { bAllowDragDrop = bEnabled; }

	/** Set whether this expression can accept drag-dropping elements onto. */
	void SetAllowDragDropOnto(bool bEnabled) { bAllowDragDropOnto = bEnabled; }

protected:

	/** Text block of the header row text. */
	TSharedPtr<class STextBlock> HeaderText;

	/** Widget before the header row text indicating expression negation state. */
	TSharedPtr<class SWidget> NegatePrefix;

	/** Box container for widgets in the right side of the header row. */
	TSharedPtr<class SHorizontalBox> ExtensionBox;

	/** Error display button. */
	TSharedPtr<class SButton> ErrorButton;

	/** Property reference of the customized expression element from the 'Expressions' array of the ActorIOExpressionContainer. */
	TSharedPtr<IPropertyHandle> PropExpression;

	/** Property reference of the 'Expressions' array of the ActorIOExpressionContainer. */
	TSharedPtr<IPropertyHandle> PropExpressionArray;

	/**
	 * Mutable struct data of the customized expression element property.
	 * This is the resolved value of the expression property, so valid as long as the property is valid.
	 */
	TInstancedStruct<FActorIOExpressionBase>* Expr;

	/** Reference to the owning container customization. */
	FActorIOExpressionContainerCustomization* ParentCustomization;

	/** Custom text to use for the header row. */
	TOptional<FText> HeaderTextOverride;

	/** Whether this expression can be removed. */
	bool bAllowRemove;

	/** Whether this expression can be drag-dropped. */
	bool bAllowDragDrop;
	
	/** Whether this expression can accept drag-dropping elements onto. */
	bool bAllowDragDropOnto;

	/** Delegate given by IDetailCustomNodeBuilder to call when child rows need to be rebuilt. */
	FSimpleDelegate OnRebuildChildren;

	/** Cached error code of the expression. */
	FString ErrorCode;

protected:

	/** Called when the error display button is clicked. */
	virtual FReply OnClick_FixErrors();

	/** Called when the remove button is clicked. */
	void OnClick_Remove();

	/** Called when the negate button is clicked. */
	void OnClick_Negate();
};

/**
 * Drag & Drop operation handler for FActorIOExpressionDetailBuilder.
 */
class ACTORIOEDITOR_API FActorIOExpressionDragDropHandler : public IDetailDragDropHandler
{
public:

	/** Constructor. */
	FActorIOExpressionDragDropHandler(FActorIOExpressionContainerCustomization* InParentCustomization, int32 InExpressionIdx, bool bInAllowDropOnto) :
		ParentCustomization(InParentCustomization),
		ExpressionIdx(InExpressionIdx),
		bAllowDropOnto(bInAllowDropOnto)
	{}

	//~ Begin IDetailDragDropHandler Interface
	virtual TSharedPtr<FDragDropOperation> CreateDragDropOperation() const override;
	virtual bool AcceptDrop(const FDragDropEvent& DragDropEvent, EItemDropZone DropZone) const override;
	virtual TOptional<EItemDropZone> CanAcceptDrop(const FDragDropEvent& DragDropEvent, EItemDropZone DropZone) const override;
	//~ End IDetailDragDropHandler Interface

private:
	
	/** Reference to the container customization that owns this expression row. */
	FActorIOExpressionContainerCustomization* ParentCustomization;

	/** Index of the expression that this row represents. */
	int32 ExpressionIdx;

	/** Whether this row can accept drag-dropping elements onto. */
	bool bAllowDropOnto;
};

/**
 * Drag & Drop operation for FActorIOExpressionDragDropHandler.
 */
class ACTORIOEDITOR_API FActorIOExpressionDragDropOp : public FDragDropOperation
{
public:

	DRAG_DROP_OPERATOR_TYPE(FActorIOExpressionDragDropOp, FDragDropOperation);

	/** Constructor. */
	FActorIOExpressionDragDropOp(int32 InExpressionIdx) :
		ExpressionIdx(InExpressionIdx)
	{}

	/** Index of the expression that is being dragged. */
	int32 ExpressionIdx;
};

/**
 * Expression row customization for FActorIOLiteralExpression.
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

	/** @return Text value of the literal. */
	FText OnGetValueText() const;

	/** Called when the literal value is committed in the text box. */
	void OnValueCommitted(const FText& InText, ETextCommit::Type InCommitType);
};

/**
 * Expression row customization for FActorIOKismetFunctionExpression.
 */
class ACTORIOEDITOR_API FActorIOKismetFunctionExpressionBuilder : public FActorIOExpressionDetailBuilder
{
public:

	/** Makes a new instance of this customization. */
	static TSharedRef<FActorIOExpressionDetailBuilder> MakeInstance();

	//~ Begin FActorIOExpressionDetailBuilder Interface
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
	virtual bool WholeRowHeader() const override { return true; }
	virtual void RefreshHeader() override;
	virtual void RefreshError() override;
	virtual FReply OnClick_FixErrors() override;
	//~ End FActorIOExpressionDetailBuilder Interface

protected:

	/** List of function names that are selectable in the function combo box. */
	TArray<FName> SelectableFunctions;

protected:

	/** @return Reference to the selected function class. */
	const UClass* OnGetFunctionClass() const;

	/** Called when a class is selected from the class combo box. */
	void OnSetFunctionClass(const UClass* SelectedClass);

	/** Called when generating an entry for the function name combo box. */
	TSharedRef<SWidget> OnGenerateFunctionComboBoxWidget(FName InName);

	/** Called before the function name combo box is opened. */
	void OnFunctionComboBoxOpening();

	/** Called when the selection changes for the function name combo box. */
	void OnFunctionComboBoxSelectionChanged(FName InName, ESelectInfo::Type InSelectType);

	/** @return Display name of the given function. */
	FText GetFunctionDisplayName(FName InFunctionName) const;

	/** @return Color based on whether the given function is valid or not. */
	FSlateColor GetFunctionDisplayColor(FName InFunctionName) const;

	/** @return Tooltip text for functions. */
	FText GetFunctionTooltip(FName InFunctionName) const;

	/** Refresh the list of selectable function names. */
	void UpdateSelectableFunctions();
};

/**
 * Expression row customization for FActorIOCompareExpression.
 */
class ACTORIOEDITOR_API FActorIOCompareExpressionBuilder : public FActorIOExpressionDetailBuilder
{
public:

	/** Makes a new instance of this customization. */
	static TSharedRef<FActorIOExpressionDetailBuilder> MakeInstance();

	//~ Begin FActorIOExpressionDetailBuilder Interface
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
	virtual bool WholeRowHeader() const override { return true; }
	virtual void RefreshHeader() override;
	virtual void RefreshError() override;
	//~ End FActorIOExpressionDetailBuilder Interface

protected:

	/** List of selectable comparison operators. */
	TArray<TSharedPtr<EActorIOComparisonOp>> ComparisonOps;

	/** Called when generating an entry for the comparison op combo box. */
	TSharedRef<SWidget> OnGenerateComparisonOpComboBoxWidget(TSharedPtr<EActorIOComparisonOp> InComparisonOp);

	/** @return Display text for the given comparison op. */
	FText GetComparisonOpDisplayName(TSharedPtr<EActorIOComparisonOp> InComparisonOp) const;

	/** @return Tooltip text for the given comparison op. */
	FText GetComparisonOpTooltip(TSharedPtr<EActorIOComparisonOp> InComparisonOp) const;

	/** Called when the selection changes for the comparison op combo box. */
	void OnComparisonOpSelectionChanged(TSharedPtr<EActorIOComparisonOp> InComparisonOp, ESelectInfo::Type InSelectType);
};

/**
 * Expression row customization for FActorIOGroupExpression.
 */
class ACTORIOEDITOR_API FActorIOGroupExpressionBuilder : public FActorIOExpressionDetailBuilder
{
public:

	/** Makes a new instance of this customization. */
	static TSharedRef<FActorIOExpressionDetailBuilder> MakeInstance();

	//~ Begin FActorIOExpressionDetailBuilder Interface
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
	virtual bool WholeRowHeader() const override { return true; }
	virtual void RefreshHeader() override;
	//~ End FActorIOExpressionDetailBuilder Interface

protected:

	/** @return Widget to display in the context menu when adding new expressions. */
	TSharedRef<SWidget> GenerateAddExpressionMenu();

	/** Called when an expression is selected from the context menu. */
	void OnClick_AddExpression(FString InType);
};

/**
 * Expression row customization for an invalid expression type.
 * @see FActorIODetailCustomizationHelper::NewExpressionDetailBuilderOfType
 */
class ACTORIOEDITOR_API FActorIOInvalidExpressionBuilder : public FActorIOExpressionDetailBuilder
{
public:

	/** Makes a new instance of this customization. */
	static TSharedRef<FActorIOExpressionDetailBuilder> MakeInstance();

	//~ Begin FActorIOExpressionDetailBuilder Interface
	virtual bool WholeRowHeader() const override { return true; }
	virtual void RefreshHeader() override;
	//~ End FActorIOExpressionDetailBuilder Interface
};

/**
 * Helper class for expression detail customizations.
 */
class ACTORIOEDITOR_API FActorIODetailCustomizationHelper
{
public:

	// #todo: create registry similar to PropertyModule.RegisterCustomClassLayout

	/**
	 * Creates a new expression row builder for the given type.
	 * If type is invalid, FActorIOInvalidExpressionBuilder is returned.
	 */
	static TSharedRef<FActorIOExpressionDetailBuilder> NewExpressionDetailBuilderOfType(FName TypeName);
};
