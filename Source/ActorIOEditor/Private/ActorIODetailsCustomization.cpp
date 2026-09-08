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

	if (Expr)
	{
		FActorIOExpressionBase* ParentExpr = Expr->GetParent();
		if (ParentExpr->GetType() == EActorIOExpressionType::Function)
		{
			FActorIOFunctionExpressionBase* FunctionExpr = static_cast<FActorIOFunctionExpressionBase*>(ParentExpr);
			FunctionExpr->RemoveArgument(Expr);

			LayoutBuilder->ForceRefreshDetails();
		}
	}
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
	ReferencedFunction = FunctionExpr ? FunctionExpr->GetUFunction() : nullptr;

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

	if (ReferencedFunction)
	{
		TArray<FProperty*> FunctionParams = FunctionExpr->GetUFunctionParams();
		for (int32 ArgIdx = 0; ArgIdx != FunctionExpr->GetArguments().Num(); ++ArgIdx)
		{
			FActorIOExpressionBase* Arg = FunctionExpr->GetArgumentAt(ArgIdx);
			if (!Arg) continue;

			TSharedRef<FActorIOExpressionDetailBuilder> Builder = FActorIODetailCustomizationHelper::NewExpressionDetailBuilderOfType(Arg->GetTypeName());
			Builder->SetExpression(Arg);
			Builder->SetNameOverrideText(FunctionParams[ArgIdx]->GetDisplayNameText());
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
			.OptionsSource(&SelectableFunctionIds)
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
		NewFunctionPtr = FunctionExpr->GetUFunction();
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
		NewFunctionPtr = FunctionExpr->GetUFunction();
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
	SelectableFunctionIds.Reset();

	FActorIOKismetFunctionExpression* FunctionExpr = GetExpression<FActorIOKismetFunctionExpression>();
	if (FunctionExpr && FunctionExpr->GetClass().IsValid())
	{
		TStrongObjectPtr<UClass> SelectedClass = FunctionExpr->GetClass().Pin();
		for (TFieldIterator<UFunction> FunctIt(SelectedClass.Get(), EFieldIteratorFlags::IncludeSuper); FunctIt; ++FunctIt)
		{
			UFunction* Function = *FunctIt;
			FProperty* ReturnProp = Function->GetReturnProperty();

			// If no return property is found, try to infer from out params (blueprint version of return values).
			// The check for NumParms == 2 will in reality check if there's only one user defined param due to a default param existing.
			if (!ReturnProp && Function->HasAnyFunctionFlags(FUNC_HasOutParms) && Function->NumParms == 2)
			{
				for (TFieldIterator<FProperty> It(Function); It && (It->PropertyFlags & CPF_OutParm); ++It)
				{
					ReturnProp = *It;
				}
			}

			// Only accept bool as return value for condition functions.
			if (CastField<FBoolProperty>(ReturnProp))
			{
				SelectableFunctionIds.AddUnique(Function->GetFName());
			}
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

	if (Expr)
	{
		FActorIOExpressionBase* ParentExpr = Expr->GetParent();
		if (ParentExpr->GetType() == EActorIOExpressionType::Function)
		{
			FActorIOFunctionExpressionBase* ParentFunctionExpr = static_cast<FActorIOFunctionExpressionBase*>(ParentExpr);
			ParentFunctionExpr->RemoveArgument(Expr);

			LayoutBuilder->ForceRefreshDetails();
		}
	}
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

	for (FActorIOExpressionBase* Arg : GroupExpr->GetArguments())
	{
		if (!Arg) continue;

		TSharedRef<FActorIOExpressionDetailBuilder> Builder = FActorIODetailCustomizationHelper::NewExpressionDetailBuilderOfType(Arg->GetTypeName());
		Builder->SetExpression(Arg);
		ChildrenBuilder.AddCustomBuilder(Builder);
	}
}

void FActorIOGroupExpressionBuilder::OnClick_AddCondition()
{
	FActorIOGroupExpression* GroupExpr = GetExpression<FActorIOGroupExpression>();
	if (GroupExpr)
	{
		FActorIOKismetFunctionExpression* NewExpression = new FActorIOKismetFunctionExpression();
		GroupExpr->AddArgument(NewExpression);

		LayoutBuilder->ForceRefreshDetails();
	}
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

	if (Expr)
	{
		FActorIOExpressionBase* ParentExpr = Expr->GetParent();
		if (ParentExpr->GetType() == EActorIOExpressionType::Function)
		{
			FActorIOFunctionExpressionBase* ParentFunctionExpr = static_cast<FActorIOFunctionExpressionBase*>(ParentExpr);
			ParentFunctionExpr->RemoveArgument(Expr);

			LayoutBuilder->ForceRefreshDetails();
		}
	}
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
//~ Begin FActorIOScriptConditionCustomization
//=======================================================

TSharedRef<IPropertyTypeCustomization> FActorIOScriptConditionCustomization::MakeInstance()
{
	return MakeShareable(new FActorIOScriptConditionCustomization);
}

void FActorIOScriptConditionCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	PropHandle = StructPropertyHandle.ToSharedPtr();
	PropUtilities = StructCustomizationUtils.GetPropertyUtilities();

	void* RawData = nullptr;
	if (StructPropertyHandle->GetValueData(RawData) == FPropertyAccess::Success)
	{
		Struct = static_cast<FActorIOScriptCondition*>(RawData);
	}

	StructPropertyHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FActorIOScriptConditionCustomization::OnValueChanged));

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
			PropertyCustomizationHelpers::MakeAddButton(FSimpleDelegate::CreateSP(this, &FActorIOScriptConditionCustomization::OnClick_AddCondition),
				LOCTEXT("ExpressionEd_AddCondition", "Add Condition"))
		]
		+ SHorizontalBox::Slot()
		.Padding(4.0f, 1.0f, 0.0f, 1.0f)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			PropertyCustomizationHelpers::MakeDeleteButton(FSimpleDelegate::CreateSP(this, &FActorIOScriptConditionCustomization::OnClick_ResetConditions),
				LOCTEXT("ExpressionEd_ResetConditions", "Remove All Conditions"))
		]
	];
}

void FActorIOScriptConditionCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	if (Struct && Struct->GetExpression())
	{
		for (FActorIOExpressionBase* Arg : Struct->GetExpression()->GetArguments())
		{
			if (!Arg) continue;

			TSharedRef<FActorIOExpressionDetailBuilder> Builder = FActorIODetailCustomizationHelper::NewExpressionDetailBuilderOfType(Arg->GetTypeName());
			Builder->SetExpression(Arg);
			StructBuilder.AddCustomBuilder(Builder);
		}
	}
}

FText FActorIOScriptConditionCustomization::GetHeaderText()
{
	int32 NumExpressions = 0;
	if (Struct && Struct->GetExpression())
	{
		NumExpressions += Struct->GetExpression()->GetNumArguments(true);
	}

	return FText::Format(LOCTEXT("ExpressionEd_ConditionHeaderText", "{0} Condition elements"), FText::AsNumber(NumExpressions));
}

void FActorIOScriptConditionCustomization::OnClick_AddCondition()
{
	if (Struct && Struct->GetExpression())
	{
		const FScopedTransaction Transaction(LOCTEXT("AddActorIOExpression", "Add ActorIO Expression"));
		TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized = PropUtilities->GetSelectedObjects();
		for (const TWeakObjectPtr<UObject>& ObjectBeingCustomized : ObjectsBeingCustomized)
		{
			ObjectBeingCustomized.Get()->Modify();
		}

		PropHandle->NotifyPreChange();

		FActorIOGroupExpression* NewExpression = new FActorIOGroupExpression();
		Struct->GetExpression()->AddArgument(NewExpression);
		
		PropHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
	}
}

void FActorIOScriptConditionCustomization::OnClick_ResetConditions()
{
	if (Struct && Struct->GetExpression())
	{
		const FScopedTransaction Transaction(LOCTEXT("ResetActorIOExpression", "Reset ActorIO Expressions"));
		TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized = PropUtilities->GetSelectedObjects();
		for (const TWeakObjectPtr<UObject>& ObjectBeingCustomized : ObjectsBeingCustomized)
		{
			ObjectBeingCustomized.Get()->Modify();
		}

		PropHandle->NotifyPreChange();

		Struct->GetExpression()->ResetArguments();

		PropHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
	}
}

void FActorIOScriptConditionCustomization::OnValueChanged()
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
