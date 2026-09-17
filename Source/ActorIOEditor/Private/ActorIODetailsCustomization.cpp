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
#include "PropertyHandle.h"
#include "PropertyCustomizationHelpers.h"
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/NotifyHook.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "ActorIOEditor"

//=======================================================
//~ Begin FActorIOExpressionDetailBuilder
//=======================================================

FActorIOExpressionDetailBuilder::FActorIOExpressionDetailBuilder()
{
	Expr = nullptr;
	NameOverrideText = FText::GetEmpty();
	bAllowRemove = true;
}

void FActorIOExpressionDetailBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	
}

void FActorIOExpressionDetailBuilder::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	LayoutBuilder = &ChildrenBuilder.GetParentCategory().GetParentLayout();
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

	TSharedPtr<SHorizontalBox> HeaderBox = nullptr;

	NodeRow
	.NameContent()
	[
		SNew(SBox)
		.VAlign(VAlign_Center)
		[
			SAssignNew(HeaderText, STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		]
	]
	.ValueContent()
	[
		SAssignNew(HeaderBox, SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SEditableTextBox)
			.Text(OnGetValueText())
			.OnTextCommitted(FOnTextCommitted::CreateSP(this, &FActorIOLiteralExpressionBuilder::OnValueCommitted))
		]
	];

	UpdateHeaderText();

	if (bAllowRemove)
	{
		HeaderBox->AddSlot()
		.Padding(4.0f, 1.0f, 0.0f, 1.0f)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			PropertyCustomizationHelpers::MakeCustomButton(FCoreStyle::Get().GetBrush("EditableComboBox.Delete"), FSimpleDelegate::CreateSP(this, &FActorIOLiteralExpressionBuilder::OnClick_Remove),
				LOCTEXT("ExpressionEd_RemoveExpression", "Remove Expression"))
		];
	}
}

void FActorIOLiteralExpressionBuilder::UpdateHeaderText()
{
	if (!NameOverrideText.IsEmpty())
	{
		HeaderText->SetText(NameOverrideText);
		return;
	}

	HeaderText->SetText(LOCTEXT("ExpressionEd_LiteralValue", "Value"));
}

FText FActorIOLiteralExpressionBuilder::OnGetValueText()
{
	FActorIOLiteralExpression* LiteralExpr = GetExpression<FActorIOLiteralExpression>();
	if (LiteralExpr)
	{
		return FText::FromString(LiteralExpr->GetLiteralValue());
	}

	return FText::GetEmpty();
}

void FActorIOLiteralExpressionBuilder::OnValueCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	FActorIOLiteralExpression* LiteralExpr = GetExpression<FActorIOLiteralExpression>();
	if (LiteralExpr)
	{
		LiteralExpr->SetLiteralValue(InText.ToString());
	}
}

void FActorIOLiteralExpressionBuilder::OnClick_Remove()
{
	// #todo: use CustomizationUtils->GetPropertyUtilities()->EnqueueDeferredAction?

	FActorIOExpressionContainer* ExprContainer = Expr->GetContainer();
	int32 ExprIdx = ExprContainer->GetExpressionIdx(Expr);
	ExprContainer->RemoveExpression(ExprIdx);

	LayoutBuilder->ForceRefreshDetails();
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

	FActorIOKismetFunctionExpression* FunctionExpr = GetExpression<FActorIOKismetFunctionExpression>();
	ReferencedFunction = FunctionExpr ? FunctionExpr->ResolveUFunction() : nullptr;

	TSharedPtr<SHorizontalBox> HeaderBox = nullptr;

	NodeRow
	.WholeRowContent()
	[
		SAssignNew(HeaderBox, SHorizontalBox)
		+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.FillWidth(1.0f)
		[
			SAssignNew(HeaderText, STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		]
		+ SHorizontalBox::Slot()
		.Padding(4.0f, 1.0f, 0.0f, 1.0f)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			SNew(SBox)
			.Visibility(ReferencedFunction ? EVisibility::Visible : EVisibility::Collapsed)
			[
				PropertyCustomizationHelpers::MakeCustomButton(FCoreStyle::Get().GetBrush("Icons.Rotate180"), FSimpleDelegate::CreateSP(this, &FActorIOKismetFunctionExpressionBuilder::OnClick_Negate),
					LOCTEXT("ExpressionEd_ToggleNegate", "Toggle Negate"))
			]
		]
	];

	UpdateHeaderText();

	if (bAllowRemove)
	{
		HeaderBox->AddSlot()
		.Padding(4.0f, 1.0f, 0.0f, 1.0f)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			PropertyCustomizationHelpers::MakeCustomButton(FCoreStyle::Get().GetBrush("EditableComboBox.Delete"), FSimpleDelegate::CreateSP(this, &FActorIOKismetFunctionExpressionBuilder::OnClick_Remove),
				LOCTEXT("ExpressionEd_RemoveExpression", "Remove Expression"))
		];
	}
}

void FActorIOKismetFunctionExpressionBuilder::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	FActorIOExpressionDetailBuilder::GenerateChildContent(ChildrenBuilder);

	FActorIOKismetFunctionExpression* FunctionExpr = GetExpression<FActorIOKismetFunctionExpression>();
	if (!FunctionExpr) return;

	FActorIOExpressionContainer* ExprContainer = Expr->GetContainer();
	if (!ExprContainer) return;

	if (ReferencedFunction)
	{
		int32 ExprIdx = ExprContainer->GetExpressionIdx(Expr);
		TArray<FActorIOExpressionBase*> ChildExpressions = ExprContainer->GetChildExpressions(ExprIdx);

		TArray<FProperty*> FunctionParams = IActorIO::GetUFunctionInputParams(ReferencedFunction);
		for (int32 ChildIdx = 0; ChildIdx != ChildExpressions.Num(); ++ChildIdx)
		{
			if (!FunctionParams.IsValidIndex(ChildIdx)) continue;

			TSharedRef<FActorIOExpressionDetailBuilder> Builder = FActorIODetailCustomizationHelper::NewExpressionDetailBuilderOfType(Expr->GetTypeName());
			Builder->SetExpression(ChildExpressions[ChildIdx]);
			Builder->SetNameOverrideText(FunctionParams[ChildIdx]->GetDisplayNameText());
			Builder->SetAllowRemove(false);
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
				.Text(GetFunctionDisplayName(FunctionExpr->GetFunctionId()))
				.ToolTipText(GetFunctionTooltip(FunctionExpr->GetFunctionId()))
				.ColorAndOpacity(GetFunctionDisplayColor(FunctionExpr->GetFunctionId()))
			]
		];
	}
}

const UClass* FActorIOKismetFunctionExpressionBuilder::OnGetFunctionClass() const
{
	FActorIOKismetFunctionExpression* FunctionExpr = GetExpression<FActorIOKismetFunctionExpression>();
	return FunctionExpr ? FunctionExpr->GetClass().Pin().Get() : nullptr;
}

void FActorIOKismetFunctionExpressionBuilder::OnSetFunctionClass(const UClass* SelectedClass)
{
	UFunction* NewFunctionPtr = nullptr;

	FActorIOKismetFunctionExpression* FunctionExpr = GetExpression<FActorIOKismetFunctionExpression>();
	if (FunctionExpr)
	{
		FunctionExpr->SetFunctionClass(const_cast<UClass*>(SelectedClass));
		NewFunctionPtr = FunctionExpr->ResolveUFunction();
	}

	if (NewFunctionPtr != ReferencedFunction)
	{
		ReferencedFunction = NewFunctionPtr;
		OnReferencedFunctionChanged();
	}
	else
	{
		// Rebuild child rows even if nothing changed because the value we set in
		// the expression may not be the same as what is currently displayed in the UI.
		OnRebuildChildren.ExecuteIfBound();
	}
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
	UFunction* NewFunctionPtr = nullptr;

	FActorIOKismetFunctionExpression* FunctionExpr = GetExpression<FActorIOKismetFunctionExpression>();
	if (FunctionExpr)
	{
		FunctionExpr->SetFunctionId(InName);
		NewFunctionPtr = FunctionExpr->ResolveUFunction();
	}

	if (NewFunctionPtr != ReferencedFunction)
	{
		ReferencedFunction = NewFunctionPtr;
		OnReferencedFunctionChanged();
	}
	else
	{
		// Rebuild child rows even if nothing changed because the value we set in
		// the expression may not be the same as what is currently displayed in the UI.
		OnRebuildChildren.ExecuteIfBound();
	}
}

FText FActorIOKismetFunctionExpressionBuilder::GetFunctionDisplayName(FName InFunctionId) const
{
	return FText::FromName(InFunctionId);
}

FSlateColor FActorIOKismetFunctionExpressionBuilder::GetFunctionDisplayColor(FName InFunctionId) const
{
	return FSlateColor::UseForeground();
}

FText FActorIOKismetFunctionExpressionBuilder::GetFunctionTooltip(FName InFunctionId)
{
	return FText::GetEmpty();
}

void FActorIOKismetFunctionExpressionBuilder::OnReferencedFunctionChanged()
{
	UpdateHeaderText();

	// Rebuild child rows to match the new arguments.
	OnRebuildChildren.ExecuteIfBound();
}

void FActorIOKismetFunctionExpressionBuilder::UpdateHeaderText()
{
	if (!NameOverrideText.IsEmpty())
	{
		HeaderText->SetText(NameOverrideText);
		return;
	}

	FActorIOKismetFunctionExpression* FunctionExpr = GetExpression<FActorIOKismetFunctionExpression>();
	if (FunctionExpr && ReferencedFunction)
	{
		FText FuncDisplayName = ReferencedFunction->GetDisplayNameText();
		FText ClassDisplayName = FunctionExpr->GetClass().Pin()->GetDisplayNameText();
		if (FunctionExpr->IsNegated())
		{
			HeaderText->SetText(FText::Format(LOCTEXT("ExpressionEd_FuncNameNegated", "{0} ({1}) [NOT]"), FuncDisplayName, ClassDisplayName));
		}
		else
		{
			HeaderText->SetText(FText::Format(LOCTEXT("ExpressionEd_FuncName", "{0} ({1})"), FuncDisplayName, ClassDisplayName));
		}
	}
	else
	{
		HeaderText->SetText(LOCTEXT("ExpressionEd_SelectFunc", "Select a function..."));
	}
}

void FActorIOKismetFunctionExpressionBuilder::UpdateSelectableFunctions()
{
	SelectableFunctions.Reset();

	FActorIOExpressionContainer* ExprContainer = Expr->GetContainer();
	FActorIOKismetFunctionExpression* FunctionExpr = GetExpression<FActorIOKismetFunctionExpression>();
	if (!FunctionExpr) return;

	if (FunctionExpr->GetClass().IsValid())
	{
		TStrongObjectPtr<UClass> SelectedClass = FunctionExpr->GetClass().Pin();
		for (TFieldIterator<UFunction> FunctIt(SelectedClass.Get(), EFieldIteratorFlags::IncludeSuper); FunctIt; ++FunctIt)
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

			SelectableFunctions.AddUnique(FunctionPtr->GetFName());
		}
	}
}

void FActorIOKismetFunctionExpressionBuilder::OnClick_Negate()
{
	FActorIOGroupExpression* GroupExpr = GetExpression<FActorIOGroupExpression>();
	if (GroupExpr)
	{
		bool bNegate = !GroupExpr->IsNegated();
		GroupExpr->SetNegated(bNegate);
	}

	UpdateHeaderText();
}

void FActorIOKismetFunctionExpressionBuilder::OnClick_Remove()
{
	// #todo: use CustomizationUtils->GetPropertyUtilities()->EnqueueDeferredAction?

	FActorIOExpressionContainer* ExprContainer = Expr->GetContainer();
	int32 ExprIdx = ExprContainer->GetExpressionIdx(Expr);
	ExprContainer->RemoveExpression(ExprIdx);

	LayoutBuilder->ForceRefreshDetails();
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

	NodeRow
	.WholeRowContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		[
			SAssignNew(HeaderText, STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
		]
		+ SHorizontalBox::Slot()
		.Padding(4.0f, 1.0f, 0.0f, 1.0f)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			PropertyCustomizationHelpers::MakeAddButton(FSimpleDelegate::CreateSP(this, &FActorIOGroupExpressionBuilder::OnClick_AddCondition),
				LOCTEXT("ExpressionEd_AddCondition", "Add Condition"))
		]
		+ SHorizontalBox::Slot()
		.Padding(4.0f, 1.0f, 0.0f, 1.0f)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			PropertyCustomizationHelpers::MakeCustomButton(FCoreStyle::Get().GetBrush("Icons.Rotate180"), FSimpleDelegate::CreateSP(this, &FActorIOGroupExpressionBuilder::OnClick_Negate),
				LOCTEXT("ExpressionEd_ToggleNegate", "Toggle Negate"))
		]
		+ SHorizontalBox::Slot()
		.Padding(4.0f, 1.0f, 0.0f, 1.0f)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			PropertyCustomizationHelpers::MakeCustomButton(FCoreStyle::Get().GetBrush("EditableComboBox.Delete"), FSimpleDelegate::CreateSP(this, &FActorIOGroupExpressionBuilder::OnClick_Remove),
				LOCTEXT("ExpressionEd_RemoveExpression", "Remove Expression"))
		]
	];

	UpdateHeaderText();
}

void FActorIOGroupExpressionBuilder::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	FActorIOExpressionDetailBuilder::GenerateChildContent(ChildrenBuilder);

	FActorIOGroupExpression* GroupExpr = GetExpression<FActorIOGroupExpression>();
	if (!GroupExpr) return;

	FActorIOExpressionContainer* ExprContainer = Expr->GetContainer();
	if (!ExprContainer) return;

	int32 ExprIdx = ExprContainer->GetExpressionIdx(Expr);
	for (FActorIOExpressionBase* ChildExpr : ExprContainer->GetChildExpressions(ExprIdx))
	{
		TSharedRef<FActorIOExpressionDetailBuilder> Builder = FActorIODetailCustomizationHelper::NewExpressionDetailBuilderOfType(ChildExpr->GetTypeName());
		Builder->SetExpression(ChildExpr);
		ChildrenBuilder.AddCustomBuilder(Builder);
	}
}

void FActorIOGroupExpressionBuilder::OnClick_AddCondition()
{
	FActorIOGroupExpression* GroupExpr = GetExpression<FActorIOGroupExpression>();
	if (!GroupExpr) return;

	FActorIOExpressionContainer* ExprContainer = Expr->GetContainer();

	FActorIOKismetFunctionExpression NewExpr;
	TInstancedStruct<FActorIOExpressionBase> NewExprInstance;
	NewExprInstance.InitializeAs<FActorIOKismetFunctionExpression>(NewExpr);

	int32 ExprParentIdx = ExprContainer->GetExpressionParentIdx(Expr);
	ExprContainer->AddExpression(NewExprInstance, ExprParentIdx);

	LayoutBuilder->ForceRefreshDetails();
}

void FActorIOGroupExpressionBuilder::OnClick_Negate()
{
	FActorIOGroupExpression* GroupExpr = GetExpression<FActorIOGroupExpression>();
	if (GroupExpr)
	{
		bool bNegate = !GroupExpr->IsNegated();
		GroupExpr->SetNegated(bNegate);
	}

	UpdateHeaderText();
}

void FActorIOGroupExpressionBuilder::OnClick_Remove()
{
	// #todo: use CustomizationUtils->GetPropertyUtilities()->EnqueueDeferredAction?

	FActorIOExpressionContainer* ExprContainer = Expr->GetContainer();
	int32 ExprIdx = ExprContainer->GetExpressionIdx(Expr);
	ExprContainer->RemoveExpression(ExprIdx);

	LayoutBuilder->ForceRefreshDetails();
}

void FActorIOGroupExpressionBuilder::UpdateHeaderText()
{
	if (!NameOverrideText.IsEmpty())
	{
		HeaderText->SetText(NameOverrideText);
		return;
	}

	FActorIOGroupExpression* GroupExpr = GetExpression<FActorIOGroupExpression>();
	if (GroupExpr && GroupExpr->IsNegated())
	{
		HeaderText->SetText(LOCTEXT("ExpressionEd_GroupNameNegated", "OR"));
	}
	else
	{
		HeaderText->SetText(LOCTEXT("ExpressionEd_GroupName", "AND"));
	}
}

//=======================================================
//~ Begin FActorIOInvalidExpressionBuilder
//=======================================================

TSharedRef<FActorIOExpressionDetailBuilder> FActorIOInvalidExpressionBuilder::MakeInstance()
{
	return MakeShareable(new FActorIOInvalidExpressionBuilder);
}

void FActorIOInvalidExpressionBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	FActorIOExpressionDetailBuilder::GenerateHeaderRowContent(NodeRow);

	NodeRow
	.WholeRowContent()
	[
		SNew(SBox)
		[
			SNew(STextBlock)
			.Text(INVTEXT("Unknown or invalid expression!"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.ColorAndOpacity(FStyleColors::Error)
		]
	];
}

//=======================================================
//~ Begin FActorIOExpressionContainerCustomization
//=======================================================

TSharedRef<IPropertyTypeCustomization> FActorIOExpressionContainerCustomization::MakeInstance()
{
	return MakeShareable(new FActorIOExpressionContainerCustomization);
}

void FActorIOExpressionContainerCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	PropStruct = StructPropertyHandle.ToSharedPtr();
	PropExpressions = StructPropertyHandle->GetChildHandle(FName("Expressions"))->AsArray();
	PropUtilities = StructCustomizationUtils.GetPropertyUtilities();

	if (PropUtilities->GetSelectedObjects().Num() > 1)
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

	StructPropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FActorIOExpressionContainerCustomization::OnValueChanged));

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
			.Text(GetHeaderText())
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		+ SHorizontalBox::Slot()
		.Padding(4.0f, 1.0f, 0.0f, 1.0f)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			PropertyCustomizationHelpers::MakeAddButton(FSimpleDelegate::CreateSP(this, &FActorIOExpressionContainerCustomization::OnClick_AddCondition),
				LOCTEXT("ExpressionEd_AddCondition", "Add Condition"))
		]
		+ SHorizontalBox::Slot()
		.Padding(4.0f, 1.0f, 0.0f, 1.0f)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			PropertyCustomizationHelpers::MakeDeleteButton(FSimpleDelegate::CreateSP(this, &FActorIOExpressionContainerCustomization::OnClick_ResetConditions),
				LOCTEXT("ExpressionEd_ResetConditions", "Remove All Conditions"))
		]
	];
}

void FActorIOExpressionContainerCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	FActorIOExpressionContainer* ExprContainer = GetStructDataPtr();
	if (ExprContainer && ExprContainer->GetNumExpressions() > 0)
	{
		for (TInstancedStruct<FActorIOExpressionBase>& ExprInstance : ExprContainer->GetExpressions())
		{
			FActorIOExpressionBase* ExprPtr = ExprInstance.GetMutablePtr<FActorIOExpressionBase>();
			TSharedRef<FActorIOExpressionDetailBuilder> Builder = FActorIODetailCustomizationHelper::NewExpressionDetailBuilderOfType(ExprPtr->GetTypeName());
			Builder->SetExpression(ExprPtr);
			StructBuilder.AddCustomBuilder(Builder);
		}
	}
}

FActorIOExpressionContainer* FActorIOExpressionContainerCustomization::GetStructDataPtr()
{
	void* RawData = nullptr;
	if (PropStruct->GetValueData(RawData) == FPropertyAccess::Success)
	{
		return static_cast<FActorIOExpressionContainer*>(RawData);
	}

	return nullptr;
}

FText FActorIOExpressionContainerCustomization::GetHeaderText()
{
	uint32 NumExpressions = 0;
	PropExpressions->GetNumElements(NumExpressions);

	return FText::Format(LOCTEXT("ExpressionEd_ConditionHeaderText", "{0} Condition elements"), FText::AsNumber(NumExpressions));
}

void FActorIOExpressionContainerCustomization::OnClick_AddCondition()
{
	FActorIOExpressionContainer* ExprContainer = GetStructDataPtr();
	if (ExprContainer)
	{
		TArray<UObject*> OuterObjects;
		PropStruct->GetOuterObjects(OuterObjects);

		const FScopedTransaction Transaction(LOCTEXT("AddActorIOExpression", "Add ActorIO Expression"));
		for (UObject* OuterObject : OuterObjects)
		{
			OuterObject->Modify();
		}

		PropStruct->NotifyPreChange();

		FActorIOGroupExpression NewExpr;
		TInstancedStruct<FActorIOExpressionBase> NewExprInstance;
		NewExprInstance.InitializeAs<FActorIOGroupExpression>(NewExpr);

		int32 ExprParentIdx = ExprContainer->GetNumExpressions() == 0 ? INDEX_NONE : 0;
		ExprContainer->AddExpression(NewExprInstance, ExprParentIdx);

		PropStruct->NotifyPostChange(EPropertyChangeType::ValueSet);
	}
}

void FActorIOExpressionContainerCustomization::OnClick_ResetConditions()
{
	FActorIOExpressionContainer* ExprContainer = GetStructDataPtr();
	if (ExprContainer)
	{
		TArray<UObject*> OuterObjects;
		PropStruct->GetOuterObjects(OuterObjects);

		const FScopedTransaction Transaction(LOCTEXT("ResetActorIOExpression", "Reset ActorIO Expressions"));
		for (UObject* OuterObject : OuterObjects)
		{
			OuterObject->Modify();
		}

		PropStruct->NotifyPreChange();

		ExprContainer->Empty();

		PropStruct->NotifyPostChange(EPropertyChangeType::ValueSet);
	}
}

void FActorIOExpressionContainerCustomization::OnValueChanged()
{
	PropUtilities->RequestForceRefresh();
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
	else if (TypeName == FName("Group"))
	{
		return FActorIOGroupExpressionBuilder::MakeInstance();
	}
	
	return FActorIOInvalidExpressionBuilder::MakeInstance();
}

#undef LOCTEXT_NAMESPACE
