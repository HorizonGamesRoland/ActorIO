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
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "ActorIOEditor"

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
	PropExpressionArray = StructPropertyHandle->GetChildHandle(FName("Expressions"));
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

	PropStruct->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FActorIOExpressionContainerCustomization::OnContainerDataChanged));
	PropExpressionArray->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FActorIOExpressionContainerCustomization::OnExpressionArrayChanged));

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
			PropertyCustomizationHelpers::MakeAddButton(FSimpleDelegate::CreateSP(this, &FActorIOExpressionContainerCustomization::OnClick_AddExpression),
				LOCTEXT("ExpressionEd_AddExpression", "Add Expression"))
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
	FActorIOExpressionContainer* ExprContainer = GetContainerData();
	if (!ExprContainer) return;

	int32 RootIdx = ExprContainer->GetRootExpressionIdx();
	if (RootIdx == INDEX_NONE) return;

	FActorIOExpressionBase* RootExpr = ExprContainer->GetExpressionPtr(RootIdx);
	TSharedPtr<IPropertyHandle> PropRootExpr = GetExpressionProperty(RootIdx);

	TSharedRef<FActorIOExpressionDetailBuilder> Builder = FActorIODetailCustomizationHelper::NewExpressionDetailBuilderOfType(RootExpr->GetTypeName());
	Builder->SetExpression(PropRootExpr, RootExpr);
	Builder->SetParentCustomization(this);
	StructBuilder.AddCustomBuilder(Builder);
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
	uint32 NumExpressions = 0;
	PropExpressionArray->AsArray()->GetNumElements(NumExpressions);

	if (Idx < (int32)NumExpressions)
	{
		return PropExpressionArray->AsArray()->GetElement(Idx);
	}

	return nullptr;
}

FText FActorIOExpressionContainerCustomization::GetHeaderText() const
{
	uint32 NumExpressions = 0;
	PropExpressionArray->AsArray()->GetNumElements(NumExpressions);

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

void FActorIOExpressionContainerCustomization::OnContainerDataChanged()
{
	if (PropUtilities.IsValid())
	{
		PropUtilities->RequestForceRefresh();
	}
}

void FActorIOExpressionContainerCustomization::OnExpressionArrayChanged()
{
	if (PropUtilities.IsValid())
	{
		PropUtilities->RequestForceRefresh();
	}
}

void FActorIOExpressionContainerCustomization::OnClick_AddExpression()
{
	FActorIOExpressionContainer* ExprContainer = GetContainerData();
	if (!ExprContainer) return;

	TArray<UObject*> OuterObjects;
	PropExpressionArray->GetOuterObjects(OuterObjects);

	const FScopedTransaction Transaction(LOCTEXT("AddActorIOExpression", "Add ActorIO Expression"));
	for (UObject* OuterObject : OuterObjects)
	{
		OuterObject->Modify();
	}

	PropExpressionArray->NotifyPreChange();

	FActorIOGroupExpression NewExpr;
	TInstancedStruct<FActorIOExpressionBase> NewExprInstance;
	NewExprInstance.InitializeAs<FActorIOGroupExpression>(NewExpr);

	int32 ExprParentIdx = ExprContainer->GetRootExpression() ? 0 : INDEX_NONE;
	ExprContainer->AddExpression(NewExprInstance, ExprParentIdx);

	PropExpressionArray->NotifyPostChange(EPropertyChangeType::ArrayAdd);
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

	ExprContainer->Empty();

	PropExpressionArray->NotifyPostChange(EPropertyChangeType::ArrayClear);
}

void FActorIOExpressionContainerCustomization::OnClick_DebugExpressions()
{
	FActorIOExpressionContainer* ExprContainer = GetContainerData();
	UE_LOG(LogTemp, Log, TEXT("%s: %s"), ANSI_TO_TCHAR(__FUNCTION__), *ExprContainer->ToString());
}

//=======================================================
//~ Begin FActorIOExpressionDetailBuilder
//=======================================================

FActorIOExpressionDetailBuilder::FActorIOExpressionDetailBuilder()
{
	Expr = nullptr;
	ParentCustomization = nullptr;
	bAllowRemove = true;
}

void FActorIOExpressionDetailBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	if (WholeRowHeader())
	{
		NodeRow
		.WholeRowContent()
		[
			SNew(SHorizontalBox)
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
			SNew(SBox)
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
	if (!HeaderTextOverride.IsEmpty())
	{
		HeaderText->SetText(HeaderTextOverride);
	}
}

void FActorIOExpressionDetailBuilder::SetExpression(const TSharedPtr<IPropertyHandle>& InProperty, FActorIOExpressionBase* InExpr)
{
	PropExpression = InProperty;
	PropExpressionArray = InProperty->GetParentHandle();
	Expr = InExpr;
}

void FActorIOExpressionDetailBuilder::OnClick_Remove()
{
	FActorIOExpressionContainer* ExprContainer = Expr->GetContainer();
	if (!ExprContainer) return;

	TArray<UObject*> OuterObjects;
	PropExpressionArray->GetOuterObjects(OuterObjects);

	const FScopedTransaction Transaction(LOCTEXT("RemoveActorIOExpression", "Remove ActorIO Expression"));
	for (UObject* OuterObject : OuterObjects)
	{
		OuterObject->Modify();
	}

	PropExpressionArray->NotifyPreChange();

	int32 ExprIdx = ExprContainer->GetExpressionIdx(Expr);
	ExprContainer->RemoveExpression(ExprIdx);

	PropExpressionArray->NotifyPostChange(EPropertyChangeType::ArrayRemove);
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

	if (HeaderTextOverride.IsEmpty())
	{
		HeaderText->SetText(LOCTEXT("ExpressionEd_LiteralValue", "Literal Value"));
	}
}

FText FActorIOLiteralExpressionBuilder::OnGetValueText() const
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

	ExtensionBox->InsertSlot(0)
	.Padding(4.0f, 1.0f, 0.0f, 1.0f)
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Center)
	.AutoWidth()
	[
		PropertyCustomizationHelpers::MakeCustomButton(FCoreStyle::Get().GetBrush("Icons.Rotate180"), FSimpleDelegate::CreateSP(this, &FActorIOKismetFunctionExpressionBuilder::OnClick_Negate),
			LOCTEXT("ExpressionEd_Negate", "Negate Expression"))
	];
}

void FActorIOKismetFunctionExpressionBuilder::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	FActorIOKismetFunctionExpression* FunctionExpr = GetExpression<FActorIOKismetFunctionExpression>();
	if (!FunctionExpr) return;

	FActorIOExpressionContainer* ExprContainer = Expr->GetContainer();
	if (!ExprContainer) return;

	UFunction* FunctionPtr = FunctionExpr->ResolveUFunction();
	if (FunctionPtr)
	{
		int32 ExprIdx = ExprContainer->GetExpressionIdx(Expr);
		TArray<int32> ChildExpressionIdxs = ExprContainer->GetChildExpressionIdxs(ExprIdx);

		TArray<FProperty*> FunctionParams = IActorIO::GetUFunctionInputParams(FunctionPtr);
		for (int32 Idx = 0; Idx != ChildExpressionIdxs.Num(); ++Idx)
		{
			if (!FunctionParams.IsValidIndex(Idx)) continue;

			FActorIOExpressionBase* ChildExpr = ExprContainer->GetExpressionPtr(ChildExpressionIdxs[Idx]);
			TSharedPtr<IPropertyHandle> PropChildExpr = ParentCustomization->GetExpressionProperty(ChildExpressionIdxs[Idx]);

			TSharedRef<FActorIOExpressionDetailBuilder> Builder = FActorIODetailCustomizationHelper::NewExpressionDetailBuilderOfType(ChildExpr->GetTypeName());
			Builder->SetExpression(PropChildExpr, ChildExpr);
			Builder->SetParentCustomization(ParentCustomization);
			Builder->SetHeaderTextOverride(FunctionParams[Idx]->GetDisplayNameText());
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

	FActorIOKismetFunctionExpression* FunctionExpr = GetExpression<FActorIOKismetFunctionExpression>();
	if (!FunctionExpr) return;

	UFunction* FunctionPtr = FunctionExpr->ResolveUFunction();
	if (FunctionPtr)
	{
		FText ClassDisplayName = FunctionExpr->GetFunctionClass()->GetDisplayNameText();
		FText FuncDisplayName = FunctionPtr->GetDisplayNameText();
		FText FuncTooltip = FunctionPtr->GetToolTipText();
		if (FuncTooltip.IdenticalTo(FuncDisplayName, ETextIdenticalModeFlags::DeepCompare))
		{
			FuncTooltip = FText::GetEmpty();
		}

		if (FunctionExpr->IsNegated())
		{
			HeaderText->SetText(FText::Format(LOCTEXT("ExpressionEd_FuncNameNegated", "[NOT] {0} ({1})"), FuncDisplayName, ClassDisplayName));
			HeaderText->SetToolTipText(FuncTooltip);
		}
		else
		{
			HeaderText->SetText(FText::Format(LOCTEXT("ExpressionEd_FuncName", "{0} ({1})"), FuncDisplayName, ClassDisplayName));
			HeaderText->SetToolTipText(FuncTooltip);
		}
	}
	else
	{
		HeaderText->SetText(LOCTEXT("ExpressionEd_SelectFunc", "Select a function..."));
		HeaderText->SetToolTipText(FText::GetEmpty());
	}
}

const UClass* FActorIOKismetFunctionExpressionBuilder::OnGetFunctionClass() const
{
	FActorIOKismetFunctionExpression* FunctionExpr = GetExpression<FActorIOKismetFunctionExpression>();
	return FunctionExpr ? FunctionExpr->GetFunctionClass() : nullptr;
}

void FActorIOKismetFunctionExpressionBuilder::OnSetFunctionClass(const UClass* SelectedClass)
{
	FActorIOKismetFunctionExpression* FunctionExpr = GetExpression<FActorIOKismetFunctionExpression>();
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

	FunctionExpr->SetFunctionClass(const_cast<UClass*>(SelectedClass));

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
	FActorIOKismetFunctionExpression* FunctionExpr = GetExpression<FActorIOKismetFunctionExpression>();
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
	return FSlateColor::UseForeground();
}

FText FActorIOKismetFunctionExpressionBuilder::GetFunctionTooltip(FName InFunctionName)
{
	FActorIOKismetFunctionExpression* FunctionExpr = GetExpression<FActorIOKismetFunctionExpression>();
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

	FActorIOExpressionContainer* ExprContainer = Expr->GetContainer();
	if (!ExprContainer) return;

	FActorIOKismetFunctionExpression* FunctionExpr = GetExpression<FActorIOKismetFunctionExpression>();
	if (!FunctionExpr) return;

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

			SelectableFunctions.AddUnique(FunctionPtr->GetFName());
		}
	}
}

void FActorIOKismetFunctionExpressionBuilder::OnClick_Negate()
{
	FActorIOKismetFunctionExpression* FunctionExpr = GetExpression<FActorIOKismetFunctionExpression>();
	if (!FunctionExpr) return;

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
//~ Begin FActorIOGroupExpressionBuilder
//=======================================================

TSharedRef<FActorIOExpressionDetailBuilder> FActorIOGroupExpressionBuilder::MakeInstance()
{
	return MakeShareable(new FActorIOGroupExpressionBuilder);
}

void FActorIOGroupExpressionBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	FActorIOExpressionDetailBuilder::GenerateHeaderRowContent(NodeRow);

	ExtensionBox->InsertSlot(0)
	.Padding(4.0f, 1.0f, 0.0f, 1.0f)
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Center)
	.AutoWidth()
	[
		PropertyCustomizationHelpers::MakeAddButton(FSimpleDelegate::CreateSP(this, &FActorIOGroupExpressionBuilder::OnClick_AddExpression),
			LOCTEXT("ExpressionEd_AddExpression", "Add Expression"))
	];

	ExtensionBox->InsertSlot(1)
	.Padding(4.0f, 1.0f, 0.0f, 1.0f)
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Center)
	.AutoWidth()
	[
		PropertyCustomizationHelpers::MakeCustomButton(FCoreStyle::Get().GetBrush("Icons.Rotate180"), FSimpleDelegate::CreateSP(this, &FActorIOGroupExpressionBuilder::OnClick_Negate),
			LOCTEXT("ExpressionEd_Negate", "Negate Expression"))
	];
}

void FActorIOGroupExpressionBuilder::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	FActorIOGroupExpression* GroupExpr = GetExpression<FActorIOGroupExpression>();
	if (!GroupExpr) return;

	FActorIOExpressionContainer* ExprContainer = Expr->GetContainer();
	if (!ExprContainer) return;

	int32 ExprIdx = ExprContainer->GetExpressionIdx(Expr);
	for (int32 ChildIdx : ExprContainer->GetChildExpressionIdxs(ExprIdx))
	{
		FActorIOExpressionBase* ChildExpr = ExprContainer->GetExpressionPtr(ChildIdx);
		TSharedPtr<IPropertyHandle> PropChildExpr = ParentCustomization->GetExpressionProperty(ChildIdx);

		TSharedRef<FActorIOExpressionDetailBuilder> Builder = FActorIODetailCustomizationHelper::NewExpressionDetailBuilderOfType(ChildExpr->GetTypeName());
		Builder->SetExpression(PropChildExpr, ChildExpr);
		Builder->SetParentCustomization(ParentCustomization);
		ChildrenBuilder.AddCustomBuilder(Builder);
	}
}

void FActorIOGroupExpressionBuilder::RefreshHeader()
{
	FActorIOExpressionDetailBuilder::RefreshHeader();

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

void FActorIOGroupExpressionBuilder::OnClick_AddExpression()
{
	FActorIOExpressionContainer* ExprContainer = Expr->GetContainer();
	if (!ExprContainer) return;

	TArray<UObject*> OuterObjects;
	PropExpressionArray->GetOuterObjects(OuterObjects);

	const FScopedTransaction Transaction(LOCTEXT("AddActorIOExpression", "Add ActorIO Expression"));
	for (UObject* OuterObject : OuterObjects)
	{
		OuterObject->Modify();
	}

	PropExpressionArray->NotifyPreChange();

	FActorIOKismetFunctionExpression NewExpr;
	TInstancedStruct<FActorIOExpressionBase> NewExprInstance;
	NewExprInstance.InitializeAs<FActorIOKismetFunctionExpression>(NewExpr);

	int32 ExprIdx = ExprContainer->GetExpressionIdx(Expr);
	ExprContainer->AddExpression(NewExprInstance, ExprIdx);

	PropExpressionArray->NotifyPostChange(EPropertyChangeType::ValueSet);
}

void FActorIOGroupExpressionBuilder::OnClick_Negate()
{
	FActorIOGroupExpression* GroupExpr = GetExpression<FActorIOGroupExpression>();
	if (!GroupExpr) return;

	TArray<UObject*> OuterObjects;
	PropExpression->GetOuterObjects(OuterObjects);

	const FScopedTransaction Transaction(LOCTEXT("NegateActorIOExpression", "Negate ActorIO Expression"));
	for (UObject* OuterObject : OuterObjects)
	{
		OuterObject->Modify();
	}
	
	PropExpression->NotifyPreChange();

	bool bNegate = !GroupExpr->IsNegated();
	GroupExpr->SetNegated(bNegate);

	PropExpression->NotifyPostChange(EPropertyChangeType::ValueSet);

	RefreshHeader();
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
	else if (TypeName == FName("Group"))
	{
		return FActorIOGroupExpressionBuilder::MakeInstance();
	}
	
	return FActorIOInvalidExpressionBuilder::MakeInstance();
}

#undef LOCTEXT_NAMESPACE
