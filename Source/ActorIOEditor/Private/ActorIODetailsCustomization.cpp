// Copyright 2024-2026 Horizon Games and all contributors at https://github.com/HorizonGamesRoland/ActorIO/graphs/contributors

#include "ActorIODetailsCustomization.h"
#include "ActorIOEditorStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "PropertyHandle.h"
#include "PropertyCustomizationHelpers.h"
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "ActorIOEditor"

//=======================================================
//~ Begin FActorIODetailCustomizationHelper
//=======================================================

TSharedRef<IDetailCustomNodeBuilder> FActorIODetailCustomizationHelper::GenerateExpressionDetailRow(const FActorIOExpressionBuilderParams& InParams)
{
	const FName ExprTypeName = InParams.Expr->GetTypeName();
	if (ExprTypeName == FName("Literal"))
	{
		return MakeShared<FActorIOLiteralExpressionBuilder>(InParams);
	}
	else if (ExprTypeName == FName("KismetFunction"))
	{
		return MakeShared<FActorIOKismetFunctionExpressionBuilder>(InParams);
	}
	else if (ExprTypeName == FName("Group"))
	{
		return MakeShared<FActorIOGroupExpressionBuilder>(InParams);
	}
	
	return MakeShared<FActorIOInvalidExpressionBuilder>(InParams);
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
	void* RawData = nullptr;
	if (StructPropertyHandle->GetValueData(RawData) == FPropertyAccess::Success)
	{
		Struct = static_cast<FActorIOScriptCondition*>(RawData);
	}

	PropUtilities = StructCustomizationUtils.GetPropertyUtilities();

	HeaderRow
	.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.MinWidth(120.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ExpressionEd_ConditionHeaderText", "3 Condition elements"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		+SHorizontalBox::Slot()
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

			FActorIOExpressionBuilderParams BuilderParams;
			BuilderParams.Expr = Arg;
			StructBuilder.AddCustomBuilder(FActorIODetailCustomizationHelper::GenerateExpressionDetailRow(BuilderParams));
		}
	}
}

void FActorIOScriptConditionCustomization::OnClick_AddCondition()
{
	if (Struct && Struct->GetExpression())
	{
		FActorIOGroupExpression* NewExpression = new FActorIOGroupExpression();
		Struct->GetExpression()->AddArgument(NewExpression);
		PropUtilities->RequestForceRefresh();
	}
}

void FActorIOScriptConditionCustomization::OnClick_ResetConditions()
{
	if (Struct && Struct->GetExpression())
	{
		Struct->GetExpression()->ResetArguments();
		PropUtilities->RequestForceRefresh();
	}
}

//=======================================================
//~ Begin FActorIOLiteralExpressionBuilder
//=======================================================

FActorIOLiteralExpressionBuilder::FActorIOLiteralExpressionBuilder(const FActorIOExpressionBuilderParams& InParams)
{
	Params = InParams;
	Expr = static_cast<FActorIOLiteralExpression*>(InParams.Expr);
}

void FActorIOLiteralExpressionBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
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
			.Text(Expr ? FText::FromString(Expr->GetLiteralValue()) : FText::GetEmpty())
			.OnTextCommitted(FOnTextCommitted::CreateSP(this, &FActorIOLiteralExpressionBuilder::OnValueCommitted))
		]
	];

	UpdateHeaderText();

	if (Params.bCanRemoveExpression)
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
	if (!Params.HeaderNameOverrideText.IsEmpty())
	{
		HeaderText->SetText(Params.HeaderNameOverrideText);
		return;
	}

	HeaderText->SetText(LOCTEXT("ExpressionEd_LiteralValue", "Value"));
}

void FActorIOLiteralExpressionBuilder::OnValueCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	if (Expr)
	{
		Expr->SetLiteralValue(InText.ToString());
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
		}
	}
}

//=======================================================
//~ Begin FActorIOKismetFunctionExpressionBuilder
//=======================================================

FActorIOKismetFunctionExpressionBuilder::FActorIOKismetFunctionExpressionBuilder(const FActorIOExpressionBuilderParams& InParams)
{
	Params = InParams;
	Expr = static_cast<FActorIOKismetFunctionExpression*>(InParams.Expr);
}

void FActorIOKismetFunctionExpressionBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
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
	];

	ReferencedFunction = Expr ? Expr->GetUFunction() : nullptr;
	UpdateHeaderText();

	if (Params.bCanRemoveExpression)
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
	LayoutBuilder = &ChildrenBuilder.GetParentCategory().GetParentLayout();

	if (Expr && ReferencedFunction)
	{
		TArray<FProperty*> FunctionParams = Expr->GetUFunctionParams();
		for (int32 ArgIdx = 0; ArgIdx != Expr->GetArguments().Num(); ++ArgIdx)
		{
			FActorIOExpressionBase* Arg = Expr->GetArgumentAt(ArgIdx);
			if (!Arg) continue;

			FActorIOExpressionBuilderParams BuilderParams;
			BuilderParams.Expr = Arg;
			BuilderParams.HeaderNameOverrideText = FunctionParams[ArgIdx]->GetDisplayNameText();
			BuilderParams.bCanRemoveExpression = false;
			ChildrenBuilder.AddCustomBuilder(FActorIODetailCustomizationHelper::GenerateExpressionDetailRow(BuilderParams));
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
			SNew(SEditableTextBox)
			.OnTextCommitted(this, &FActorIOKismetFunctionExpressionBuilder::OnSetFunctionId)
		];
	}
}

const UClass* FActorIOKismetFunctionExpressionBuilder::OnGetFunctionClass() const
{
	return Expr ? Expr->GetClass().Pin().Get() : nullptr;
}

void FActorIOKismetFunctionExpressionBuilder::OnSetFunctionClass(const UClass* SelectedClass)
{
	UFunction* NewFunctionPtr = nullptr;
	if (Expr)
	{
		Expr->SetFunctionClass(const_cast<UClass*>(SelectedClass));
		NewFunctionPtr = Expr->GetUFunction();
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

FName FActorIOKismetFunctionExpressionBuilder::OnGetFunctionId()
{
	return Expr ? Expr->GetFunctionId() : NAME_None;
}

void FActorIOKismetFunctionExpressionBuilder::OnSetFunctionId(const FText& NewText, ETextCommit::Type CommitInfo)
{
	UFunction* NewFunctionPtr = nullptr;
	if (Expr)
	{
		Expr->SetFunctionId(FName(NewText.ToString(), EFindName::FNAME_Find));
		NewFunctionPtr = Expr->GetUFunction();
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

void FActorIOKismetFunctionExpressionBuilder::OnReferencedFunctionChanged()
{
	UpdateHeaderText();

	// Rebuild child rows to match the new arguments.
	OnRebuildChildren.ExecuteIfBound();
}

void FActorIOKismetFunctionExpressionBuilder::UpdateHeaderText()
{
	if (!Params.HeaderNameOverrideText.IsEmpty())
	{
		HeaderText->SetText(Params.HeaderNameOverrideText);
		return;
	}

	if (ReferencedFunction)
	{
		HeaderText->SetText(ReferencedFunction->GetDisplayNameText());
	}
	else
	{
		HeaderText->SetText(LOCTEXT("ExpressionEd_SelectFunc", "Select a function..."));
	}
}

void FActorIOKismetFunctionExpressionBuilder::OnClick_Remove()
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
//~ Begin FActorIOGroupExpressionBuilder
//=======================================================

FActorIOGroupExpressionBuilder::FActorIOGroupExpressionBuilder(const FActorIOExpressionBuilderParams& InParams)
{
	Params = InParams;
	Expr = static_cast<FActorIOGroupExpression*>(InParams.Expr);
}

void FActorIOGroupExpressionBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	NodeRow
	.WholeRowContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SBox)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ExpressionEd_GroupName", "If all of the following is TRUE:"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(2.0f)
		[
			SNew(SButton)
			.OnClicked(FOnClicked::CreateSP(this, &FActorIOGroupExpressionBuilder::OnAddFunctionClicked))
			[
				SNew(STextBlock)
				.Text(INVTEXT("Add Expression"))
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 2.0f)
		[
			SNew(SButton)
			.OnClicked(FOnClicked::CreateSP(this, &FActorIOGroupExpressionBuilder::OnAddGroupClicked))
			[
				SNew(STextBlock)
				.Text(INVTEXT("Add Group"))
			]
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
}

void FActorIOGroupExpressionBuilder::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	for (FActorIOExpressionBase* Arg : Expr->GetArguments())
	{
		if (!Arg) continue;

		FActorIOExpressionBuilderParams BuilderParams;
		BuilderParams.Expr = Arg;
		ChildrenBuilder.AddCustomBuilder(FActorIODetailCustomizationHelper::GenerateExpressionDetailRow(BuilderParams));
	}
}

FReply FActorIOGroupExpressionBuilder::OnAddFunctionClicked()
{
	FActorIOKismetFunctionExpression* NewExpression = new FActorIOKismetFunctionExpression();
	Expr->AddArgument(NewExpression);

	OnRebuildChildren.ExecuteIfBound();
	return FReply::Handled();
}

FReply FActorIOGroupExpressionBuilder::OnAddGroupClicked()
{
	FActorIOGroupExpression* NewExpression = new FActorIOGroupExpression();
	Expr->AddArgument(NewExpression);

	OnRebuildChildren.ExecuteIfBound();
	return FReply::Handled();
}

void FActorIOGroupExpressionBuilder::OnClick_Remove()
{
	// #todo: use CustomizationUtils->GetPropertyUtilities()->EnqueueDeferredAction?

	if (Expr)
	{
		FActorIOExpressionBase* ParentExpr = Expr->GetParent();
		if (ParentExpr->GetType() == EActorIOExpressionType::Function)
		{
			FActorIOFunctionExpressionBase* FunctionExpr = static_cast<FActorIOFunctionExpressionBase*>(ParentExpr);
			FunctionExpr->RemoveArgument(Expr);
		}
	}
}

//=======================================================
//~ Begin FActorIOInvalidExpressionBuilder
//=======================================================

FActorIOInvalidExpressionBuilder::FActorIOInvalidExpressionBuilder(const FActorIOExpressionBuilderParams& InParams)
{
	Params = InParams;
}

void FActorIOInvalidExpressionBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	NodeRow
	.WholeRowContent()
	[
		SNew(SBox)
		[
			SNew(STextBlock)
			.Text(INVTEXT("Unknown Expression!"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.ColorAndOpacity(FStyleColors::Error)
		]
	];
}

#undef LOCTEXT_NAMESPACE
