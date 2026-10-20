// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIODetailsCustomization.h"
#include "ActorIOEditorStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "PropertyHandle.h"
#include "PropertyCustomizationHelpers.h"
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Editor.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "ActorIOEditor"

//=======================================================
//~ Begin FActorIOExpressionContainerCustomization
//=======================================================

FActorIOExpressionContainerCustomization::FActorIOExpressionContainerCustomization()
{
	bRootExpressionHidden = false;
}

FActorIOExpressionContainerCustomization::~FActorIOExpressionContainerCustomization()
{
	FActorIOExpressionContainer* ExprContainer = GetContainerData();
	if (ExprContainer && DelegateHandle_FixupContainerReferences.IsValid())
	{
		ExprContainer->OnFixupContainerReferences().Remove(DelegateHandle_FixupContainerReferences);
		DelegateHandle_FixupContainerReferences.Reset();
	}

	if (DelegateHandle_BlueprintCompiled.IsValid() && GEditor)
	{
		GEditor->OnBlueprintCompiled().Remove(DelegateHandle_BlueprintCompiled);
		DelegateHandle_BlueprintCompiled.Reset();
	}
}

TSharedRef<IPropertyTypeCustomization> FActorIOExpressionContainerCustomization::MakeInstance()
{
	return MakeShareable(new FActorIOExpressionContainerCustomization);
}

void FActorIOExpressionContainerCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	PropStruct = StructPropertyHandle.ToSharedPtr();
	PropExpressionArray = StructPropertyHandle->GetChildHandle(FName("Expressions"));
	PropUtilities = StructCustomizationUtils.GetPropertyUtilities();

	if (PropStruct->GetNumOuterObjects() > 1)
	{
		HeaderRow
		.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(SBox)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ExpressionEd_MultiSelectError", "Can't edit multiple objects"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			]
		];
		return;
	}

	PropStruct->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FActorIOExpressionContainerCustomization::RefreshDetails));
	PropExpressionArray->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FActorIOExpressionContainerCustomization::RefreshDetails));

	bRootExpressionHidden = PropStruct->HasMetaData("HideRootExpression");

	FActorIOExpressionContainer* ExprContainer = GetContainerData();
	if (!DelegateHandle_FixupContainerReferences.IsValid() && ExprContainer)
	{
		DelegateHandle_FixupContainerReferences = ExprContainer->OnFixupContainerReferences().AddSP(this, &FActorIOExpressionContainerCustomization::RefreshDetails);
	}

	if (!DelegateHandle_BlueprintCompiled.IsValid() && GEditor)
	{
		DelegateHandle_BlueprintCompiled = GEditor->OnBlueprintCompiled().AddSP(this, &FActorIOExpressionContainerCustomization::RefreshDetails);
	}

	HeaderRow
	.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.MinWidth(120.0f)
		[
			SNew(STextBlock)
			.Text(this, &FActorIOExpressionContainerCustomization::GetHeaderText)
			.ToolTipText(this, &FActorIOExpressionContainerCustomization::GetHeaderTooltip)
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		+ SHorizontalBox::Slot()
		.Padding(4.0f, 1.0f, 0.0f, 1.0f)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(SComboButton)
			.ComboButtonStyle(FAppStyle::Get(), "SimpleComboButtonWithIcon")
			.HasDownArrow(false)
			.MenuPlacement(EMenuPlacement::MenuPlacement_CenteredAboveAnchor)
			.MenuContent()
			[
				GenerateAddExpressionMenu()
			]
			.ButtonContent()
			[
				SNew(SImage)
				.Image(FAppStyle::GetBrush("Icons.PlusCircle"))
				.ColorAndOpacity(FSlateColor::UseForeground())
				.ToolTipText(LOCTEXT("ExpressionEd_AddExpression", "Add Expression"))
			]
		]
		+ SHorizontalBox::Slot()
		.Padding(4.0f, 1.0f, 0.0f, 1.0f)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			PropertyCustomizationHelpers::MakeDeleteButton(FSimpleDelegate::CreateSP(this, &FActorIOExpressionContainerCustomization::OnClick_ClearExpressions),
				LOCTEXT("ExpressionEd_ClearExpressions", "Remove All Expressions"))
		]
		+ SHorizontalBox::Slot()
		.Padding(4.0f, 1.0f, 0.0f, 1.0f)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			PropertyCustomizationHelpers::MakeCustomButton(FCoreStyle::Get().GetBrush("Icons.Info"), FSimpleDelegate::CreateSP(this, &FActorIOExpressionContainerCustomization::OnClick_DebugExpressions),
				LOCTEXT("ExpressionEd_DebugExpression", "Debug Expression"))
		]
	];
}

void FActorIOExpressionContainerCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	if (PropStruct->GetNumOuterObjects() > 1) return;

	FActorIOExpressionContainer* ExprContainer = GetContainerData();
	if (!ExprContainer) return;

	int32 RootIdx = ExprContainer->GetRootExpressionIdx();
	if (RootIdx == INDEX_NONE) return;

	if (bRootExpressionHidden)
	{
		for (int32 ChildIdx : ExprContainer->GetChildExpressionIdxs(RootIdx))
		{
			TInstancedStruct<FActorIOExpressionBase>& ChildExpr = ExprContainer->GetExpression(ChildIdx);
			TSharedPtr<IPropertyHandle> PropChildExpr = GetExpressionProperty(ChildIdx);

			TSharedRef<FActorIOExpressionDetailBuilder> Builder = FActorIODetailCustomizationHelper::NewExpressionDetailBuilderOfType(ChildExpr->GetTypeName());
			Builder->SetExpression(PropChildExpr, &ChildExpr);
			Builder->SetParentCustomization(this);
			StructBuilder.AddCustomBuilder(Builder);
		}
	}
	else
	{
		TInstancedStruct<FActorIOExpressionBase>& RootExpr = ExprContainer->GetExpression(RootIdx);
		TSharedPtr<IPropertyHandle> PropRootExpr = GetExpressionProperty(RootIdx);

		TSharedRef<FActorIOExpressionDetailBuilder> Builder = FActorIODetailCustomizationHelper::NewExpressionDetailBuilderOfType(RootExpr->GetTypeName());
		Builder->SetExpression(PropRootExpr, &RootExpr);
		Builder->SetParentCustomization(this);
		StructBuilder.AddCustomBuilder(Builder);
	}
}

void FActorIOExpressionContainerCustomization::RefreshDetails()
{
	if (PropUtilities.IsValid())
	{
		PropUtilities->RequestForceRefresh();
	}
}

FActorIOExpressionContainer* FActorIOExpressionContainerCustomization::GetContainerData() const
{
	void* RawData = nullptr;
	if (PropStruct.IsValid() && PropStruct->GetValueData(RawData) == FPropertyAccess::Success)
	{
		return static_cast<FActorIOExpressionContainer*>(RawData);
	}

	return nullptr;
}

TSharedPtr<IPropertyHandle> FActorIOExpressionContainerCustomization::GetExpressionProperty(int32 Idx) const
{
	if (PropExpressionArray.IsValid() && Idx >= 0)
	{
		TSharedPtr<IPropertyHandleArray> PropArray = PropExpressionArray->AsArray();
		uint32 NumExpressions = 0;
		PropArray->GetNumElements(NumExpressions);

		if (Idx < (int32)NumExpressions)
		{
			return PropArray->GetElement(Idx);
		}
	}

	return nullptr;
}

FText FActorIOExpressionContainerCustomization::GetHeaderText() const
{
	uint32 NumExpressions = 0;
	PropExpressionArray->AsArray()->GetNumElements(NumExpressions);

	if (bRootExpressionHidden && NumExpressions > 0)
	{
		NumExpressions--;
	}

	return FText::Format(LOCTEXT("ExpressionEd_ContainerHeaderText", "{0} Expression elements"), FText::AsNumber(NumExpressions));
}

FText FActorIOExpressionContainerCustomization::GetHeaderTooltip() const
{
	FActorIOExpressionContainer* ExprContainer = GetContainerData();
	if (ExprContainer)
	{
		return FText::FromString(ExprContainer->ToString());
	}

	return FText::GetEmpty();
}

TSharedRef<SWidget> FActorIOExpressionContainerCustomization::GenerateAddExpressionMenu()
{
	FActorIOExpressionContainer* ExprContainer = GetContainerData();
	if (!ExprContainer) return SNullWidget::NullWidget;

	FMenuBuilder MenuBuilder(true, NULL, NULL);

	MenuBuilder.BeginSection(TEXT("AddExpression"), LOCTEXT("ExpressionEd_AddExpressionMenu", "Add Expression"));

	const FActorIOExpressionBase* RootExpr = ExprContainer->GetRootExpression();

	bool bCanAddNew = !RootExpr || (RootExpr->CanContainChildExpressions() && RootExpr->CanEditChildExpressions());
	if (!bCanAddNew)
	{
		MenuBuilder.AddWidget
		(
			SNew(STextBlock)
			.Text(LOCTEXT("ExpressionEd_AddExpression_CantAddNew", "The current root cannot contain new children."))
			.ColorAndOpacity(FAppStyle::GetColor("ErrorReporting.WarningBackgroundColor"))
			.Margin(FMargin(16.0f, 4.0f))
			.Font(FAppStyle::GetFontStyle("BoldFont")),
			FText::GetEmpty(),
			true
		);
	}
	
	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("ExpressionEd_AddExpression_Group", "Add Group"),
		FText::GetEmpty(),
		FSlateIcon(),
		FUIAction
		(
			FExecuteAction::CreateSP(this, &FActorIOExpressionContainerCustomization::OnClick_AddExpression, FString(TEXT("and"))),
			FCanExecuteAction::CreateLambda([bCanAddNew]() { return bCanAddNew; })
		)
	);

	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("ExpressionEd_AddExpression_Func", "Add Function"),
		FText::GetEmpty(),
		FSlateIcon(),
		FUIAction
		(
			FExecuteAction::CreateSP(this, &FActorIOExpressionContainerCustomization::OnClick_AddExpression, FString(TEXT("func"))),
			FCanExecuteAction::CreateLambda([bCanAddNew]() { return bCanAddNew; })
		)
	);

	if (ExprContainer->IsConditionContainer())
	{
		MenuBuilder.AddMenuEntry
		(
			LOCTEXT("ExpressionEd_AddExpression_Compare", "Add Compare"),
			FText::GetEmpty(),
			FSlateIcon(),
			FUIAction
			(
				FExecuteAction::CreateSP(this, &FActorIOExpressionContainerCustomization::OnClick_AddExpression, FString(TEXT("eq"))),
				FCanExecuteAction::CreateLambda([bCanAddNew]() { return bCanAddNew; })

			)
		);
	}

	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void FActorIOExpressionContainerCustomization::OnClick_AddExpression(FString InType)
{
	FActorIOExpressionContainer* ExprContainer = GetContainerData();
	if (!ExprContainer) return;

	TInstancedStruct<FActorIOExpressionBase> NewExprInstance = FActorIOExpressionParser::NewExpressionOfType(InType);
	if (!NewExprInstance.IsValid()) return;

	TArray<UObject*> OuterObjects;
	PropExpressionArray->GetOuterObjects(OuterObjects);

	const FScopedTransaction Transaction(LOCTEXT("AddActorIOExpression", "Add ActorIO Expression"));
	for (UObject* OuterObject : OuterObjects)
	{
		OuterObject->Modify();
	}

	PropExpressionArray->NotifyPreChange();

	int32 ExprParentIdx = ExprContainer->GetRootExpression() ? 0 : INDEX_NONE;
	ExprContainer->AddExpression(NewExprInstance, ExprParentIdx);

	PropExpressionArray->NotifyPostChange(EPropertyChangeType::ArrayAdd);

	PropStruct->SetExpanded(true);
}

void FActorIOExpressionContainerCustomization::OnClick_ClearExpressions()
{
	FActorIOExpressionContainer* ExprContainer = GetContainerData();
	if (!ExprContainer) return;

	TArray<UObject*> OuterObjects;
	PropExpressionArray->GetOuterObjects(OuterObjects);

	const FScopedTransaction Transaction(LOCTEXT("ClearActorIOExpression", "Clear ActorIO Expressions"));
	for (UObject* OuterObject : OuterObjects)
	{
		OuterObject->Modify();
	}

	PropExpressionArray->NotifyPreChange();

	if (bRootExpressionHidden)
	{
		int32 RootIdx = ExprContainer->GetRootExpressionIdx();
		ExprContainer->RemoveChildExpressions(RootIdx);
	}
	else
	{
		ExprContainer->Empty();
	}

	PropExpressionArray->NotifyPostChange(EPropertyChangeType::ArrayClear);
}

void FActorIOExpressionContainerCustomization::OnClick_DebugExpressions()
{
	FActorIOExpressionContainer* ExprContainer = GetContainerData();
	if (!ExprContainer) return;

	ExprContainer->DebugDumpContainerData();
}

//=======================================================
//~ Begin FActorIOExpressionDetailBuilder
//=======================================================

FActorIOExpressionDetailBuilder::FActorIOExpressionDetailBuilder()
{
	Expr = nullptr;
	ParentCustomization = nullptr;
	bAllowRemove = true;
	bAllowDragDrop = true;
	bAllowDragDropOnto = false;
}

void FActorIOExpressionDetailBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	FActorIOExpressionBase* ExprBase = Expr ? Expr->GetMutablePtr() : nullptr;
	FActorIOExpressionContainer* ExprContainer = ExprBase ? ExprBase->GetContainer() : nullptr;
	if (!ExprBase || !ExprContainer) return;

	//FActorIOExpressionBase* ExprBase = nullptr;
	//FActorIOExpressionContainer* ExprContainer = nullptr;
	//if (!GetExpressionData(ExprBase, ExprContainer))
	//{
	//	return;
	//}

	if (WholeRowHeader())
	{
		NodeRow
		.WholeRowContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SAssignNew(NegatePrefix, STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFontBold())
				.Text(LOCTEXT("ExpressionEd_NegatePrefix", "[NOT]"))
				.Margin(FMargin(0.0f, 0.0f, 5.0f, 0.0f))
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.FillWidth(1.0f)
			[
				SAssignNew(HeaderText, STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SAssignNew(ExtensionBox, SHorizontalBox)
			]
		];
	}
	else
	{
		NodeRow
		.NameContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				SAssignNew(NegatePrefix, STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFontBold())
				.Text(LOCTEXT("ExpressionEd_NegatePrefix", "[NOT]"))
				.Margin(FMargin(0.0f, 0.0f, 5.0f, 0.0f))
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SAssignNew(HeaderText, STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			]
		]
		.ExtensionContent()
		[
			SAssignNew(ExtensionBox, SHorizontalBox)
		];
	}

	if (bAllowDragDrop)
	{
		int32 ExpressionIdx = PropExpression->GetIndexInArray();
		NodeRow.DragDropHandler(MakeShared<FActorIOExpressionDragDropHandler>(ParentCustomization, ExpressionIdx, bAllowDragDropOnto));
	}

	ExtensionBox->AddSlot()
	.Padding(4.0f, 1.0f, 0.0f, 1.0f)
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Center)
	.AutoWidth()
	[
		SAssignNew(ErrorButton, SButton)
		.ButtonStyle(FAppStyle::Get(), "SimpleButton")
		.ContentPadding(0.0f)
		.OnClicked(FOnClicked::CreateSP(this, &FActorIOExpressionDetailBuilder::OnClick_FixErrors))
		[
			SNew(SImage)
			.Image(FAppStyle::GetBrush("Icons.Error.Solid"))
		]
	];

	FActorIOFunctionExpressionBase* FunctionExpr = Expr->GetMutablePtr<FActorIOFunctionExpressionBase>();
	if (FunctionExpr && ExprContainer->IsConditionContainer())
	{
		ExtensionBox->AddSlot()
		.Padding(4.0f, 1.0f, 0.0f, 1.0f)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			PropertyCustomizationHelpers::MakeCustomButton(FCoreStyle::Get().GetBrush("Icons.Rotate180"), FSimpleDelegate::CreateSP(this, &FActorIOKismetFunctionExpressionBuilder::OnClick_Negate),
				LOCTEXT("ExpressionEd_Negate", "Negate Expression"))
		];
	}

	if (bAllowRemove)
	{
		ExtensionBox->AddSlot()
		.Padding(4.0f, 1.0f, 0.0f, 1.0f)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			PropertyCustomizationHelpers::MakeCustomButton(FCoreStyle::Get().GetBrush("EditableComboBox.Delete"), FSimpleDelegate::CreateSP(this, &FActorIOExpressionDetailBuilder::OnClick_Remove),
				LOCTEXT("ExpressionEd_RemoveExpression", "Remove Expression"))
		];
	}

	RefreshHeader();
}

void FActorIOExpressionDetailBuilder::RefreshHeader()
{
	if (HeaderTextOverride.IsSet())
	{
		HeaderText->SetText(HeaderTextOverride.GetValue());
	}

	FActorIOFunctionExpressionBase* FunctionExpr = Expr ? Expr->GetMutablePtr<FActorIOFunctionExpressionBase>() : nullptr;
	if (FunctionExpr)
	{
		const bool bNegated = FunctionExpr->IsNegated();
		NegatePrefix->SetVisibility(bNegated ? EVisibility::Visible : EVisibility::Collapsed);
	}
	else
	{
		NegatePrefix->SetVisibility(EVisibility::Collapsed);
	}

	RefreshError();
}

void FActorIOExpressionDetailBuilder::RefreshError()
{
	FActorIOExpressionBase* ExprBase = Expr ? Expr->GetMutablePtr() : nullptr;
	if (ExprBase)
	{
		ErrorCode = ExprBase->CheckForErrors();
		ErrorButton->SetVisibility(ErrorCode.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible);
		ErrorButton->SetToolTipText(FText::FromString(ErrorCode));
	}
	else
	{
		ErrorCode = TEXT("ExprInvalid");
		ErrorButton->SetVisibility(EVisibility::Visible);
		ErrorButton->SetToolTipText(LOCTEXT("ExpressionEd_ExprInvalidError", "Expression reference is null!"));
	}
}

void FActorIOExpressionDetailBuilder::SetExpression(const TSharedPtr<IPropertyHandle>& InProperty, TInstancedStruct<FActorIOExpressionBase>* InExpr)
{
	PropExpression = InProperty;
	PropExpressionArray = InProperty->GetParentHandle();
	Expr = InExpr;
}

FReply FActorIOExpressionDetailBuilder::OnClick_FixErrors()
{
	return FReply::Handled();
}

void FActorIOExpressionDetailBuilder::OnClick_Remove()
{
	FActorIOExpressionBase* ExprBase = Expr ? Expr->GetMutablePtr() : nullptr;
	FActorIOExpressionContainer* ExprContainer = ExprBase ? ExprBase->GetContainer() : nullptr;
	if (!ExprBase || !ExprContainer) return;

	TArray<UObject*> OuterObjects;
	PropExpressionArray->GetOuterObjects(OuterObjects);

	const FScopedTransaction Transaction(LOCTEXT("RemoveActorIOExpression", "Remove ActorIO Expression"));
	for (UObject* OuterObject : OuterObjects)
	{
		OuterObject->Modify();
	}

	PropExpressionArray->NotifyPreChange();

	const int32 ExprIdx = ExprContainer->GetExpressionIdx(ExprBase);
	ExprContainer->RemoveExpression(ExprIdx);

	PropExpressionArray->NotifyPostChange(EPropertyChangeType::ArrayRemove);
}

void FActorIOExpressionDetailBuilder::OnClick_Negate()
{
	FActorIOFunctionExpressionBase* FunctionExpr = Expr ? Expr->GetMutablePtr<FActorIOFunctionExpressionBase>() : nullptr;
	FActorIOExpressionContainer* ExprContainer = FunctionExpr ? FunctionExpr->GetContainer() : nullptr;
	if (!FunctionExpr || !ExprContainer) return;

	TArray<UObject*> OuterObjects;
	PropExpression->GetOuterObjects(OuterObjects);

	const FScopedTransaction Transaction(LOCTEXT("NegateActorIOExpression", "Negate ActorIO Expression"));
	for (UObject* OuterObject : OuterObjects)
	{
		OuterObject->Modify();
	}

	PropExpression->NotifyPreChange();

	bool bNegate = !FunctionExpr->IsNegated();
	FunctionExpr->SetNegated(bNegate);

	PropExpression->NotifyPostChange(EPropertyChangeType::ValueSet);

	RefreshHeader();
}

//=======================================================
//~ Begin FActorIOExpressionDragDropHandler
//=======================================================

TSharedPtr<FDragDropOperation> FActorIOExpressionDragDropHandler::CreateDragDropOperation() const
{
	TSharedPtr<FActorIOExpressionDragDropOp> DragOp = MakeShared<FActorIOExpressionDragDropOp>(ExpressionIdx);
	return DragOp;
}

bool FActorIOExpressionDragDropHandler::AcceptDrop(const FDragDropEvent& DragDropEvent, EItemDropZone DropZone) const
{
	const TSharedPtr<FActorIOExpressionDragDropOp> DragOp = DragDropEvent.GetOperationAs<FActorIOExpressionDragDropOp>();
	if (!DragOp.IsValid())
	{
		return false;
	}

	FActorIOExpressionContainer* ExprContainer = ParentCustomization->GetContainerData();
	if (!ExprContainer)
	{
		return false;
	}

	int32 ParentIdx = ExprContainer->GetExpressionParentIdx(ExpressionIdx);
	int32 SlotIdx = ExprContainer->GetExpressionSlotIdx(ExpressionIdx);
	if (DropZone == EItemDropZone::OntoItem)
	{
		ParentIdx = ExpressionIdx;
		SlotIdx = INDEX_NONE;
	}

	TArray<UObject*> OuterObjects;
	ParentCustomization->GetExpressionArrayProperty()->GetOuterObjects(OuterObjects);

	const FScopedTransaction Transaction(LOCTEXT("MoveActorIOExpression", "Move ActorIO Expression"));
	for (UObject* OuterObject : OuterObjects)
	{
		OuterObject->Modify();
	}

	ParentCustomization->GetExpressionArrayProperty()->NotifyPreChange();

	ExprContainer->MoveExpressionTo(DragOp->ExpressionIdx, ParentIdx, SlotIdx);

	ParentCustomization->GetExpressionArrayProperty()->NotifyPostChange(EPropertyChangeType::ValueSet);

	return true;
}

TOptional<EItemDropZone> FActorIOExpressionDragDropHandler::CanAcceptDrop(const FDragDropEvent& DragDropEvent, EItemDropZone DropZone) const
{
	TOptional<EItemDropZone> ReturnedDropZone;

	const TSharedPtr<FActorIOExpressionDragDropOp> DragDropOp = DragDropEvent.GetOperationAs<FActorIOExpressionDragDropOp>();
	const FActorIOExpressionContainer* ExprContainer = ParentCustomization->GetContainerData();
	if (!DragDropOp.IsValid() || !ExprContainer)
	{
		return ReturnedDropZone;
	}

	if (!ExprContainer->IsValidIndex(ExpressionIdx) || !ExprContainer->IsValidIndex(DragDropOp->ExpressionIdx))
	{
		return ReturnedDropZone;
	}

	if (DropZone == EItemDropZone::OntoItem)
	{
		// Can't drop onto the expression unless allowed.
		if (!bAllowDropOnto)
		{
			return ReturnedDropZone;
		}

		// Can't drop onto our own parent.
		if (ExprContainer->GetExpressionParentIdx(DragDropOp->ExpressionIdx) == ExpressionIdx)
		{
			return ReturnedDropZone;
		}
	}
	else
	{
		// Can't drop above/below the root expression.
		if (ExpressionIdx == 0)
		{
			return ReturnedDropZone;
		}
	}

	// Can't drop above/below/onto any of our own childs.
	if (ExprContainer->IsExpressionDescendantOf(ExpressionIdx, DragDropOp->ExpressionIdx))
	{
		return ReturnedDropZone;
	}

	ReturnedDropZone = DropZone;
	return ReturnedDropZone;
}

//=======================================================
//~ Begin FActorIOLiteralExpressionBuilder
//=======================================================

TSharedRef<FActorIOExpressionDetailBuilder> FActorIOLiteralExpressionBuilder::MakeInstance()
{
	return MakeShareable(new FActorIOLiteralExpressionBuilder);
}

void FActorIOLiteralExpressionBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	FActorIOExpressionDetailBuilder::GenerateHeaderRowContent(NodeRow);

	NodeRow
	.ValueContent()
	[
		SNew(SEditableTextBox)
		.Text(OnGetValueText())
		.OnTextCommitted(FOnTextCommitted::CreateSP(this, &FActorIOLiteralExpressionBuilder::OnValueCommitted))
	];
}

void FActorIOLiteralExpressionBuilder::RefreshHeader()
{
	FActorIOExpressionDetailBuilder::RefreshHeader();

	if (!HeaderTextOverride.IsSet())
	{
		HeaderText->SetText(LOCTEXT("ExpressionEd_LiteralValue", "Literal Value"));
	}
}

FText FActorIOLiteralExpressionBuilder::OnGetValueText() const
{
	FActorIOLiteralExpression* LiteralExpr = Expr ? Expr->GetMutablePtr<FActorIOLiteralExpression>() : nullptr;
	if (!LiteralExpr) return FText::GetEmpty();

	return FText::FromString(LiteralExpr->GetLiteralValue());
}

void FActorIOLiteralExpressionBuilder::OnValueCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	FActorIOLiteralExpression* LiteralExpr = Expr ? Expr->GetMutablePtr<FActorIOLiteralExpression>() : nullptr;
	if (!LiteralExpr) return;

	TArray<UObject*> OuterObjects;
	PropExpression->GetOuterObjects(OuterObjects);

	const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOExpression", "Modify ActorIO Expression"));
	for (UObject* OuterObject : OuterObjects)
	{
		OuterObject->Modify();
	}

	PropExpression->NotifyPreChange();

	LiteralExpr->SetLiteralValue(InText.ToString());

	PropExpression->NotifyPostChange(EPropertyChangeType::ValueSet);
}

//=======================================================
//~ Begin FActorIOKismetFunctionExpressionBuilder
//=======================================================

TSharedRef<FActorIOExpressionDetailBuilder> FActorIOKismetFunctionExpressionBuilder::MakeInstance()
{
	return MakeShareable(new FActorIOKismetFunctionExpressionBuilder);
}

void FActorIOKismetFunctionExpressionBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	FActorIOExpressionDetailBuilder::GenerateHeaderRowContent(NodeRow);
}

void FActorIOKismetFunctionExpressionBuilder::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	FActorIOKismetFunctionExpression* FunctionExpr = Expr ? Expr->GetMutablePtr<FActorIOKismetFunctionExpression>() : nullptr;
	FActorIOExpressionContainer* ExprContainer = FunctionExpr ? FunctionExpr->GetContainer() : nullptr;
	if (!FunctionExpr || !ExprContainer) return;

	UFunction* FunctionPtr = FunctionExpr->ResolveUFunction();
	if (FunctionPtr)
	{
		TArray<FProperty*> FunctionParams = IActorIO::GetUFunctionInputParams(FunctionPtr);

		const int32 ExprIdx = ExprContainer->GetExpressionIdx(FunctionExpr);
		const TArray<int32> ChildExpressionIdxs = ExprContainer->GetChildExpressionIdxs(ExprIdx);
		for (int32 Idx = 0; Idx != ChildExpressionIdxs.Num(); ++Idx)
		{
			TInstancedStruct<FActorIOExpressionBase>& ChildExpr = ExprContainer->GetExpression(ChildExpressionIdxs[Idx]);
			TSharedPtr<IPropertyHandle> PropChildExpr = ParentCustomization->GetExpressionProperty(ChildExpressionIdxs[Idx]);

			FText ChildExprDisplayName = LOCTEXT("ExpressionEd_InvalidParamName", "<Invalid>");
			FString ChildExprPropName;
			if (ChildExpr->GetMetadata(TEXT("PropName"), ChildExprPropName))
			{
				for (FProperty* Param : FunctionParams)
				{
					if (Param->GetAuthoredName() == ChildExprPropName)
					{
						ChildExprDisplayName = Param->GetDisplayNameText();
						break;
					}
				}
			}

			TSharedRef<FActorIOExpressionDetailBuilder> Builder = FActorIODetailCustomizationHelper::NewExpressionDetailBuilderOfType(ChildExpr->GetTypeName());
			Builder->SetExpression(PropChildExpr, &ChildExpr);
			Builder->SetParentCustomization(ParentCustomization);
			Builder->SetHeaderTextOverride(ChildExprDisplayName);
			Builder->SetAllowRemove(false);
			Builder->SetAllowDragDrop(false);
			ChildrenBuilder.AddCustomBuilder(Builder);
		}
	}
	else
	{
		ChildrenBuilder.AddCustomRow(LOCTEXT("ExpressionEd_SelectFuncClassSearchText", "Class"))
		.NameContent()
		[
			SNew(SBox)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ExpressionEd_SelectFuncClass", "Class"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			]
		]
		.ValueContent()
		[
			SNew(SClassPropertyEntryBox)
			.MetaClass(UBlueprintFunctionLibrary::StaticClass())
			.AllowNone(false)
			.AllowAbstract(false)
			.SelectedClass(this, &FActorIOKismetFunctionExpressionBuilder::OnGetFunctionClass)
			.OnSetClass(this, &FActorIOKismetFunctionExpressionBuilder::OnSetFunctionClass)
		];

		ChildrenBuilder.AddCustomRow(LOCTEXT("ExpressionEd_SelectFuncNameSearchText", "Function"))
		.NameContent()
		[
			SNew(SBox)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ExpressionEd_SelectFuncName", "Function"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			]
		]
		.ValueContent()
		[
			SNew(SComboBox<FName>)
			.OptionsSource(&SelectableFunctions)
			.OnGenerateWidget(this, &FActorIOKismetFunctionExpressionBuilder::OnGenerateFunctionComboBoxWidget)
			.OnComboBoxOpening(this, &FActorIOKismetFunctionExpressionBuilder::OnFunctionComboBoxOpening)
			.OnSelectionChanged(this, &FActorIOKismetFunctionExpressionBuilder::OnFunctionComboBoxSelectionChanged)
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(GetFunctionDisplayName(FunctionExpr->GetFunctionName()))
				.ToolTipText(GetFunctionTooltip(FunctionExpr->GetFunctionName()))
				.ColorAndOpacity(GetFunctionDisplayColor(FunctionExpr->GetFunctionName()))
			]
		];
	}
}

void FActorIOKismetFunctionExpressionBuilder::RefreshHeader()
{
	FActorIOExpressionDetailBuilder::RefreshHeader();

	FActorIOKismetFunctionExpression* FunctionExpr = Expr ? Expr->GetMutablePtr<FActorIOKismetFunctionExpression>() : nullptr;
	if (!FunctionExpr) return;

	if (!HeaderTextOverride.IsSet())
	{
		UFunction* FunctionPtr = FunctionExpr->ResolveUFunction();
		if (FunctionPtr)
		{
			FText ClassDisplayName = FunctionExpr->GetFunctionClass()->GetDisplayNameText();
			FText FuncDisplayName = FunctionPtr->GetDisplayNameText();
			FText FuncTooltip = FunctionPtr->GetToolTipText();
			if (FuncTooltip.IdenticalTo(FuncDisplayName, ETextIdenticalModeFlags::DeepCompare))
			{
				// Do not show tooltip if its just the auto generated name of the function.
				FuncTooltip = FText::GetEmpty();
			}

			HeaderText->SetText(FText::Format(LOCTEXT("ExpressionEd_FuncName", "{0} ({1})"), FuncDisplayName, ClassDisplayName));
			HeaderText->SetToolTipText(FuncTooltip);
		}
		else
		{
			HeaderText->SetText(LOCTEXT("ExpressionEd_SelectFunc", "Select a function..."));
			HeaderText->SetToolTipText(FText::GetEmpty());
		}
	}
}

void FActorIOKismetFunctionExpressionBuilder::RefreshError()
{
	FActorIOExpressionDetailBuilder::RefreshError();

	if (ErrorCode == TEXT("FunctionNotFound"))
	{
		ErrorButton->SetToolTipText(LOCTEXT("ExpressionEd_FuncNotFoundTooltip", "The selected function cannot be found."));
	}
	else if (ErrorCode == TEXT("ArgumentsNeedUpdate"))
	{
		ErrorButton->SetToolTipText(LOCTEXT("ExpressionEd_FixChildExpressionsTooltip", "Child expressions are out of date. Click to fix."));
	}
}

FReply FActorIOKismetFunctionExpressionBuilder::OnClick_FixErrors()
{
	FActorIOKismetFunctionExpression* FunctionExpr = Expr ? Expr->GetMutablePtr<FActorIOKismetFunctionExpression>() : nullptr;
	if (!FunctionExpr) return FReply::Handled();

	if (ErrorCode == TEXT("FunctionNotFound"))
	{
		// Can't really fix. User needs to select a new function.
	}
	else if (ErrorCode == TEXT("ArgumentsNeedUpdate"))
	{
		TArray<UObject*> OuterObjects;
		PropExpressionArray->GetOuterObjects(OuterObjects);

		const FScopedTransaction Transaction(LOCTEXT("FixActorIOExpression", "Fix ActorIO Expression"));
		for (UObject* OuterObject : OuterObjects)
		{
			OuterObject->Modify();
		}

		PropExpression->NotifyPreChange();
		PropExpressionArray->NotifyPreChange();

		FunctionExpr->UpdateArguments();

		PropExpression->NotifyPostChange(EPropertyChangeType::ValueSet);
		PropExpressionArray->NotifyPostChange(EPropertyChangeType::ValueSet);
	}

	return FReply::Handled();
}

const UClass* FActorIOKismetFunctionExpressionBuilder::OnGetFunctionClass() const
{
	FActorIOKismetFunctionExpression* FunctionExpr = Expr ? Expr->GetMutablePtr<FActorIOKismetFunctionExpression>() : nullptr;
	return FunctionExpr ? FunctionExpr->GetFunctionClass() : nullptr;
}

void FActorIOKismetFunctionExpressionBuilder::OnSetFunctionClass(const UClass* SelectedClass)
{
	FActorIOKismetFunctionExpression* FunctionExpr = Expr ? Expr->GetMutablePtr<FActorIOKismetFunctionExpression>() : nullptr;
	if (!FunctionExpr) return;

	TArray<UObject*> OuterObjects;
	PropExpressionArray->GetOuterObjects(OuterObjects);

	const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOExpression", "Modify ActorIO Expression"));
	for (UObject* OuterObject : OuterObjects)
	{
		OuterObject->Modify();
	}

	PropExpression->NotifyPreChange();
	PropExpressionArray->NotifyPreChange();

	FunctionExpr->SetFunctionClass(const_cast<UClass*>(SelectedClass), true);

	PropExpression->NotifyPostChange(EPropertyChangeType::ValueSet);
	PropExpressionArray->NotifyPostChange(EPropertyChangeType::ValueSet);
}

TSharedRef<SWidget> FActorIOKismetFunctionExpressionBuilder::OnGenerateFunctionComboBoxWidget(FName InName)
{
	return SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(GetFunctionDisplayName(InName))
		.ToolTipText(GetFunctionTooltip(InName));
}

void FActorIOKismetFunctionExpressionBuilder::OnFunctionComboBoxOpening()
{
	UpdateSelectableFunctions();
}

void FActorIOKismetFunctionExpressionBuilder::OnFunctionComboBoxSelectionChanged(FName InName, ESelectInfo::Type InSelectType)
{
	FActorIOKismetFunctionExpression* FunctionExpr = Expr ? Expr->GetMutablePtr<FActorIOKismetFunctionExpression>() : nullptr;
	if (!FunctionExpr) return;

	TArray<UObject*> OuterObjects;
	PropExpressionArray->GetOuterObjects(OuterObjects);

	const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOExpression", "Modify ActorIO Expression"));
	for (UObject* OuterObject : OuterObjects)
	{
		OuterObject->Modify();
	}

	PropExpression->NotifyPreChange();
	PropExpressionArray->NotifyPreChange();

	FunctionExpr->SetFunctionName(InName);

	PropExpression->NotifyPostChange(EPropertyChangeType::ValueSet);
	PropExpressionArray->NotifyPostChange(EPropertyChangeType::ValueSet);
}

FText FActorIOKismetFunctionExpressionBuilder::GetFunctionDisplayName(FName InFunctionName) const
{
	return FText::FromName(InFunctionName);
}

FSlateColor FActorIOKismetFunctionExpressionBuilder::GetFunctionDisplayColor(FName InFunctionName) const
{
	FActorIOKismetFunctionExpression* FunctionExpr = Expr ? Expr->GetMutablePtr<FActorIOKismetFunctionExpression>() : nullptr;
	if (FunctionExpr)
	{
		const UClass* ClassPtr = FunctionExpr->GetFunctionClass();
		const UFunction* FunctionPtr = ClassPtr ? ClassPtr->FindFunctionByName(InFunctionName) : nullptr;
		if (ClassPtr && !FunctionPtr && InFunctionName != NAME_None)
		{
			return FStyleColors::Error;
		}
	}

	return FSlateColor::UseForeground();
}

FText FActorIOKismetFunctionExpressionBuilder::GetFunctionTooltip(FName InFunctionName) const
{
	FActorIOKismetFunctionExpression* FunctionExpr = Expr ? Expr->GetMutablePtr<FActorIOKismetFunctionExpression>() : nullptr;
	if (FunctionExpr)
	{
		const UClass* ClassPtr = FunctionExpr->GetFunctionClass();
		const UFunction* FunctionPtr = ClassPtr ? ClassPtr->FindFunctionByName(InFunctionName) : nullptr;
		if (FunctionPtr)
		{
			return FunctionPtr->GetToolTipText();
		}
	}

	return FText::GetEmpty();
}

void FActorIOKismetFunctionExpressionBuilder::UpdateSelectableFunctions()
{
	SelectableFunctions.Reset();

	FActorIOKismetFunctionExpression* FunctionExpr = Expr ? Expr->GetMutablePtr<FActorIOKismetFunctionExpression>() : nullptr;
	FActorIOExpressionContainer* ExprContainer = FunctionExpr ? FunctionExpr->GetContainer() : nullptr;
	if (!FunctionExpr || !ExprContainer) return;

	const UClass* ClassPtr = FunctionExpr->GetFunctionClass();
	if (ClassPtr)
	{
		for (TFieldIterator<UFunction> FunctIt(ClassPtr, EFieldIteratorFlags::IncludeSuper); FunctIt; ++FunctIt)
		{
			UFunction* FunctionPtr = *FunctIt;

			if (ExprContainer->IsConditionContainer())
			{
				FProperty* ReturnProp = IActorIO::GetUFunctionReturnProperty(FunctionPtr);
				if (!CastField<FBoolProperty>(ReturnProp))
				{
					continue;
				}
			}
			else
			{
				if (!FunctionPtr->HasAnyFunctionFlags(FUNC_BlueprintCallable) || FunctionPtr->HasAnyFunctionFlags(FUNC_Delegate | FUNC_BlueprintPure))
				{
					continue;
				}
			}

			SelectableFunctions.AddUnique(FunctionPtr->GetFName());
		}
	}
}

//=======================================================
//~ Begin FActorIOGroupExpressionBuilder
//=======================================================

TSharedRef<FActorIOExpressionDetailBuilder> FActorIOCompareExpressionBuilder::MakeInstance()
{
	return MakeShareable(new FActorIOCompareExpressionBuilder);
}

void FActorIOCompareExpressionBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	FActorIOExpressionDetailBuilder::GenerateHeaderRowContent(NodeRow);

	ComparisonOps.Empty();
	for (int64 Index = 0; Index < StaticEnum<EActorIOComparisonOp>()->GetMaxEnumValue(); ++Index)
	{
		ComparisonOps.Add(MakeShareable(new EActorIOComparisonOp(static_cast<EActorIOComparisonOp>(Index))));
	}
}

void FActorIOCompareExpressionBuilder::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	FActorIOCompareExpression* CompareExpr = Expr ? Expr->GetMutablePtr<FActorIOCompareExpression>() : nullptr;
	FActorIOExpressionContainer* ExprContainer = CompareExpr ? CompareExpr->GetContainer() : nullptr;
	if (!CompareExpr || !ExprContainer) return;

	const int32 ExprIdx = ExprContainer->GetExpressionIdx(CompareExpr);
	const TArray<int32> ChildExpressionIdxs = ExprContainer->GetChildExpressionIdxs(ExprIdx);
	if (ChildExpressionIdxs.Num() != 2)
	{
		return;
	}

	{
		TInstancedStruct<FActorIOExpressionBase>& ChildExpr = ExprContainer->GetExpression(ChildExpressionIdxs[0]);
		TSharedPtr<IPropertyHandle> PropChildExpr = ParentCustomization->GetExpressionProperty(ChildExpressionIdxs[0]);

		TSharedRef<FActorIOExpressionDetailBuilder> Builder = FActorIODetailCustomizationHelper::NewExpressionDetailBuilderOfType(ChildExpr->GetTypeName());
		Builder->SetExpression(PropChildExpr, &ChildExpr);
		Builder->SetParentCustomization(ParentCustomization);
		Builder->SetHeaderTextOverride(LOCTEXT("ExpressionEd_ComparisonValueA", "Value A"));
		Builder->SetAllowRemove(false);
		Builder->SetAllowDragDrop(false);
		ChildrenBuilder.AddCustomBuilder(Builder);
	}

	{
		int32 CurrentComparisonOpIdx = 0;
		for (int32 OpIdx = 0; OpIdx != ComparisonOps.Num(); ++OpIdx)
		{
			if (CompareExpr->GetComparisonOp() == *ComparisonOps[OpIdx])
			{
				CurrentComparisonOpIdx = OpIdx;
				break;
			}
		}

		FDetailWidgetRow& Row = ChildrenBuilder.AddCustomRow(LOCTEXT("ExpressionEd_ComparisonOpSearchText", "Comparison Operator"));
		Row.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			.Text(LOCTEXT("ExpressionEd_ComparisonOpName", "Comparison Operator"))
		]
		.ValueContent()
		[
			SNew(SComboBox<TSharedPtr<EActorIOComparisonOp>>)
			.OptionsSource(&ComparisonOps)
			.InitiallySelectedItem(ComparisonOps[CurrentComparisonOpIdx])
			.OnGenerateWidget(this, &FActorIOCompareExpressionBuilder::OnGenerateComparisonOpComboBoxWidget)
			.OnSelectionChanged(this, &FActorIOCompareExpressionBuilder::OnComparisonOpSelectionChanged)
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(GetComparisonOpDisplayName(ComparisonOps[CurrentComparisonOpIdx]))
				.ToolTipText(GetComparisonOpTooltip(ComparisonOps[CurrentComparisonOpIdx]))
			]
		];
	}

	{
		TInstancedStruct<FActorIOExpressionBase>& ChildExpr = ExprContainer->GetExpression(ChildExpressionIdxs[1]);
		TSharedPtr<IPropertyHandle> PropChildExpr = ParentCustomization->GetExpressionProperty(ChildExpressionIdxs[1]);

		TSharedRef<FActorIOExpressionDetailBuilder> Builder = FActorIODetailCustomizationHelper::NewExpressionDetailBuilderOfType(ChildExpr->GetTypeName());
		Builder->SetExpression(PropChildExpr, &ChildExpr);
		Builder->SetParentCustomization(ParentCustomization);
		Builder->SetHeaderTextOverride(LOCTEXT("ExpressionEd_ComparisonValueB", "Value B"));
		Builder->SetAllowRemove(false);
		Builder->SetAllowDragDrop(false);
		ChildrenBuilder.AddCustomBuilder(Builder);
	}
}

void FActorIOCompareExpressionBuilder::RefreshHeader()
{
	FActorIOExpressionDetailBuilder::RefreshHeader();

	if (!HeaderTextOverride.IsSet())
	{
		HeaderText->SetText(LOCTEXT("ExpressionEd_CompareName", "Compare"));
	}
}

void FActorIOCompareExpressionBuilder::RefreshError()
{
	FActorIOExpressionDetailBuilder::RefreshError();

	if (ErrorCode == TEXT("InvalidArgumentCount"))
	{
		ErrorButton->SetToolTipText(LOCTEXT("ExpressionEd_CompareInvalidArgCountTooltip", "Child expression count is invalid! You most likely have to re-create this expression."));
	}
}

TSharedRef<SWidget> FActorIOCompareExpressionBuilder::OnGenerateComparisonOpComboBoxWidget(TSharedPtr<EActorIOComparisonOp> InComparisonOp)
{
	return SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(GetComparisonOpDisplayName(InComparisonOp))
		.ToolTipText(GetComparisonOpTooltip(InComparisonOp));
}

FText FActorIOCompareExpressionBuilder::GetComparisonOpDisplayName(TSharedPtr<EActorIOComparisonOp> InComparisonOp) const
{
	switch (*InComparisonOp)
	{
	case EActorIOComparisonOp::Less:
		return FText::FromString(TEXT("<"));
	case EActorIOComparisonOp::LessOrEqual:
		return FText::FromString(TEXT("<="));
	case EActorIOComparisonOp::Equal:
		return FText::FromString(TEXT("=="));
	case EActorIOComparisonOp::NotEqual:
		return FText::FromString(TEXT("!="));
	case EActorIOComparisonOp::GreaterOrEqual:
		return FText::FromString(TEXT(">="));
	case EActorIOComparisonOp::Greater:
		return FText::FromString(TEXT(">"));
	default:
		return FText::FromString(TEXT("??"));
	}
}

FText FActorIOCompareExpressionBuilder::GetComparisonOpTooltip(TSharedPtr<EActorIOComparisonOp> InComparisonOp) const
{
	switch (*InComparisonOp)
	{
	case EActorIOComparisonOp::Less:
		return LOCTEXT("ExpressionEd_ComparisonOpTooltip_Less", "Less Than");
	case EActorIOComparisonOp::LessOrEqual:
		return LOCTEXT("ExpressionEd_ComparisonOpTooltip_LessOrEqual", "Less Than or Equal");
	case EActorIOComparisonOp::Equal:
		return LOCTEXT("ExpressionEd_ComparisonOpTooltip_Equal", "Equal");
	case EActorIOComparisonOp::NotEqual:
		return LOCTEXT("ExpressionEd_ComparisonOpTooltip_NotEqual", "Not Equal");
	case EActorIOComparisonOp::GreaterOrEqual:
		return LOCTEXT("ExpressionEd_ComparisonOpTooltip_GreaterOrEqual", "Greater Than or Equal");
	case EActorIOComparisonOp::Greater:
		return LOCTEXT("ExpressionEd_ComparisonOpTooltip_Greater", "Greater Than");
	default:
		return FText::GetEmpty();
	}
}

void FActorIOCompareExpressionBuilder::OnComparisonOpSelectionChanged(TSharedPtr<EActorIOComparisonOp> InComparisonOp, ESelectInfo::Type InSelectType)
{
	FActorIOCompareExpression* CompareExpr = Expr ? Expr->GetMutablePtr<FActorIOCompareExpression>() : nullptr;
	if (!CompareExpr || !InComparisonOp.IsValid()) return;

	TArray<UObject*> OuterObjects;
	PropExpression->GetOuterObjects(OuterObjects);

	const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOExpression", "Modify ActorIO Expression"));
	for (UObject* OuterObject : OuterObjects)
	{
		OuterObject->Modify();
	}

	PropExpression->NotifyPreChange();

	CompareExpr->SetComparisonOp(*InComparisonOp);

	PropExpression->NotifyPostChange(EPropertyChangeType::ValueSet);
}

//=======================================================
//~ Begin FActorIOGroupExpressionBuilder
//=======================================================

TSharedRef<FActorIOExpressionDetailBuilder> FActorIOGroupExpressionBuilder::MakeInstance()
{
	return MakeShareable(new FActorIOGroupExpressionBuilder);
}

void FActorIOGroupExpressionBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	FActorIOExpressionDetailBuilder::GenerateHeaderRowContent(NodeRow);

	ExtensionBox->InsertSlot(1)
	.Padding(4.0f, 1.0f, 0.0f, 1.0f)
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Center)
	.AutoWidth()
	[
		SNew(SComboButton)
		.ComboButtonStyle(FAppStyle::Get(), "SimpleComboButtonWithIcon")
		.HasDownArrow(false)
		.MenuPlacement(EMenuPlacement::MenuPlacement_CenteredAboveAnchor)
		.MenuContent()
		[
			GenerateAddExpressionMenu()
		]
		.ButtonContent()
		[
			SNew(SImage)
			.Image(FAppStyle::GetBrush("Icons.PlusCircle"))
			.ColorAndOpacity(FSlateColor::UseForeground())
			.ToolTipText(LOCTEXT("ExpressionEd_AddExpression", "Add Expression"))
		]
	];
}

void FActorIOGroupExpressionBuilder::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	FActorIOGroupExpression* GroupExpr = Expr ? Expr->GetMutablePtr<FActorIOGroupExpression>() : nullptr;
	FActorIOExpressionContainer* ExprContainer = GroupExpr ? GroupExpr->GetContainer() : nullptr;
	if (!GroupExpr || !ExprContainer) return;

	const int32 ExprIdx = ExprContainer->GetExpressionIdx(GroupExpr);
	for (int32 ChildIdx : ExprContainer->GetChildExpressionIdxs(ExprIdx))
	{
		TInstancedStruct<FActorIOExpressionBase>& ChildExpr = ExprContainer->GetExpression(ChildIdx);
		TSharedPtr<IPropertyHandle> PropChildExpr = ParentCustomization->GetExpressionProperty(ChildIdx);

		TSharedRef<FActorIOExpressionDetailBuilder> Builder = FActorIODetailCustomizationHelper::NewExpressionDetailBuilderOfType(ChildExpr->GetTypeName());
		Builder->SetExpression(PropChildExpr, &ChildExpr);
		Builder->SetParentCustomization(ParentCustomization);
		ChildrenBuilder.AddCustomBuilder(Builder);
	}
}

void FActorIOGroupExpressionBuilder::RefreshHeader()
{
	FActorIOExpressionDetailBuilder::RefreshHeader();

	if (!HeaderTextOverride.IsSet())
	{
		if (NegatePrefix.IsValid())
		{
			NegatePrefix->SetVisibility(EVisibility::Collapsed);
		}

		FActorIOGroupExpression* GroupExpr = Expr ? Expr->GetMutablePtr<FActorIOGroupExpression>() : nullptr;
		if (GroupExpr && GroupExpr->IsNegated())
		{
			HeaderText->SetText(LOCTEXT("ExpressionEd_GroupNameNegated", "OR"));
			HeaderText->SetToolTipText(LOCTEXT("ExpressionEd_GroupTooltipNegated", "A group that is true if *Any* of its childs are true."));
		}
		else
		{
			HeaderText->SetText(LOCTEXT("ExpressionEd_GroupName", "AND"));
			HeaderText->SetToolTipText(LOCTEXT("ExpressionEd_GroupTooltip", "A group that is true if *All* of its childs are true."));
		}
	}
}

TSharedRef<SWidget> FActorIOGroupExpressionBuilder::GenerateAddExpressionMenu()
{
	FActorIOGroupExpression* GroupExpr = Expr ? Expr->GetMutablePtr<FActorIOGroupExpression>() : nullptr;
	FActorIOExpressionContainer* ExprContainer = GroupExpr ? GroupExpr->GetContainer() : nullptr;
	if (!GroupExpr || !ExprContainer) return SNullWidget::NullWidget;

	FMenuBuilder MenuBuilder(true, NULL, NULL);

	MenuBuilder.BeginSection(TEXT("ExpressionType"), LOCTEXT("ExpressionEd_AddExpressionMenu", "Add Expression"));

	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("ExpressionEd_AddExpression_Group", "Add Group"),
		FText::GetEmpty(),
		FSlateIcon(),
		FExecuteAction::CreateSP(this, &FActorIOGroupExpressionBuilder::OnClick_AddExpression, FString(TEXT("and")))
	);

	MenuBuilder.AddMenuEntry
	(
		LOCTEXT("ExpressionEd_AddExpression_Func", "Add Function"),
		FText::GetEmpty(),
		FSlateIcon(),
		FExecuteAction::CreateSP(this, &FActorIOGroupExpressionBuilder::OnClick_AddExpression, FString(TEXT("func")))
	);

	if (ExprContainer->IsConditionContainer())
	{
		MenuBuilder.AddMenuEntry
		(
			LOCTEXT("ExpressionEd_AddExpression_Compare", "Add Compare"),
			FText::GetEmpty(),
			FSlateIcon(),
			FExecuteAction::CreateSP(this, &FActorIOGroupExpressionBuilder::OnClick_AddExpression, FString(TEXT("eq")))
		);
	}

	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

void FActorIOGroupExpressionBuilder::OnClick_AddExpression(FString InType)
{
	FActorIOGroupExpression* GroupExpr = Expr ? Expr->GetMutablePtr<FActorIOGroupExpression>() : nullptr;
	FActorIOExpressionContainer* ExprContainer = GroupExpr ? GroupExpr->GetContainer() : nullptr;
	if (!GroupExpr || !ExprContainer) return;

	TInstancedStruct<FActorIOExpressionBase> NewExprInstance = FActorIOExpressionParser::NewExpressionOfType(InType);
	if (!NewExprInstance.IsValid()) return;

	TArray<UObject*> OuterObjects;
	PropExpressionArray->GetOuterObjects(OuterObjects);

	const FScopedTransaction Transaction(LOCTEXT("AddActorIOExpression", "Add ActorIO Expression"));
	for (UObject* OuterObject : OuterObjects)
	{
		OuterObject->Modify();
	}

	PropExpressionArray->NotifyPreChange();

	const int32 ExprIdx = ExprContainer->GetExpressionIdx(GroupExpr);
	ExprContainer->AddExpression(NewExprInstance, ExprIdx);

	PropExpressionArray->NotifyPostChange(EPropertyChangeType::ValueSet);
}

//=======================================================
//~ Begin FActorIOInvalidExpressionBuilder
//=======================================================

TSharedRef<FActorIOExpressionDetailBuilder> FActorIOInvalidExpressionBuilder::MakeInstance()
{
	return MakeShareable(new FActorIOInvalidExpressionBuilder);
}

void FActorIOInvalidExpressionBuilder::RefreshHeader()
{
	FActorIOExpressionDetailBuilder::RefreshHeader();

	HeaderText->SetText(LOCTEXT("ExpressionEd_InvalidExpression", "Unknown expression!"));
	HeaderText->SetColorAndOpacity(FStyleColors::Error);
}

//=======================================================
//~ Begin FActorIODetailCustomizationHelper
//=======================================================

TSharedRef<FActorIOExpressionDetailBuilder> FActorIODetailCustomizationHelper::NewExpressionDetailBuilderOfType(FName TypeName)
{
	if (TypeName == FName("Literal"))
	{
		return FActorIOLiteralExpressionBuilder::MakeInstance();
	}
	else if (TypeName == FName("KismetFunction"))
	{
		return FActorIOKismetFunctionExpressionBuilder::MakeInstance();
	}
	else if (TypeName == FName("Compare"))
	{
		return FActorIOCompareExpressionBuilder::MakeInstance();
	}
	else if (TypeName == FName("Group"))
	{
		TSharedRef<FActorIOExpressionDetailBuilder> GroupBuilder = FActorIOGroupExpressionBuilder::MakeInstance();
		GroupBuilder->SetAllowDragDropOnto(true);
		return GroupBuilder;
	}
	
	return FActorIOInvalidExpressionBuilder::MakeInstance();
}

#undef LOCTEXT_NAMESPACE
