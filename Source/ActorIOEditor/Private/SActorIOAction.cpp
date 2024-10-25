// Copyright 2024 Horizon Games. All Rights Reserved.

#include "SActorIOAction.h"
#include "ActorIOComponent.h"
#include "ActorIOTypes.h"

#define LOCTEXT_NAMESPACE "FActorIOEditor"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SActorIOAction::Construct(const FArguments& InArgs)
{
	IOComponent = InArgs._IOComponent;
	ActionIdx = InArgs._ActionIdx;

	const FActorIOAction& Action = GetAction();

	UpdateSelectableEvents();
	UpdateSelectableFunctions();

	ChildSlot
	[
		SNew(SBox)
		.HeightOverride(30.0f)
		[
			SNew(SSplitter)
			+ SSplitter::Slot()
			[
				SNew(SComboBox<FName>)
				.OptionsSource(&SelectableEvents)
				.OnGenerateWidget(this, &SActorIOAction::OnGenerateComboBoxWidget)
				.OnSelectionChanged(this, &SActorIOAction::OnSelectedEventChanged)
				[
					SAssignNew(EventText, STextBlock)
					.Text(FText::FromName(Action.SourceEvent))
				]
			]
			+ SSplitter::Slot()
			[
				SNew(SComboBox<FName>)
				.OptionsSource(&SelectableEvents)
				.OnGenerateWidget(this, &SActorIOAction::OnGenerateComboBoxWidget)
				.OnSelectionChanged(this, &SActorIOAction::OnSelectedEventChanged)
				[
					SAssignNew(TargetActorText, STextBlock)
					.Text(FText::FromString(Action.TargetActor ? Action.TargetActor->GetActorNameOrLabel() : TEXT("")))
				]
			]
			+ SSplitter::Slot()
			[
				SNew(SComboBox<FName>)
				.OptionsSource(&SelectableFunctions)
				.OnGenerateWidget(this, &SActorIOAction::OnGenerateComboBoxWidget)
				.OnSelectionChanged(this, &SActorIOAction::OnSelectedFunctionChanged)
				[
					SAssignNew(FunctionText, STextBlock)
					.Text(FText::FromName(Action.TargetFunction))
				]
			]
		]
	];
}

void SActorIOAction::RebuildWidget()
{
	UpdateSelectableEvents();
	UpdateSelectableFunctions();

	const FActorIOAction& Action = IOComponent->GetActions()[ActionIdx];
	EventText->SetText(FText::FromName(Action.SourceEvent));
	TargetActorText->SetText(FText::FromString(Action.TargetActor ? Action.TargetActor->GetActorNameOrLabel() : TEXT("")));
	FunctionText->SetText(FText::FromName(Action.TargetFunction));
}

TSharedRef<SWidget> SActorIOAction::OnGenerateComboBoxWidget(FName InName)
{
	return SNew(STextBlock)
		.Text(FText::FromName(InName));
}

void SActorIOAction::OnSelectedEventChanged(FName InName, ESelectInfo::Type InSelectType)
{
	if (InSelectType != ESelectInfo::Direct)
	{
		const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
		IOComponent->Modify();

		FActorIOAction& TargetAction = GetAction();
		TargetAction.SourceEvent = InName;

		RebuildWidget();
	}
}

void SActorIOAction::OnSelectedFunctionChanged(FName InName, ESelectInfo::Type InSelectType)
{
	if (InSelectType != ESelectInfo::Direct)
	{
		const FScopedTransaction Transaction(LOCTEXT("ModifyActorIOAction", "Modify ActorIO Action"));
		IOComponent->Modify();

		FActorIOAction& TargetAction = GetAction();
		TargetAction.TargetFunction = InName;

		RebuildWidget();
	}
}

void SActorIOAction::UpdateSelectableEvents()
{
	SelectableEvents.Reset();
	SelectableEvents.Add(TEXT("<Clear>"));

	TArray<FActorIOEvent> ValidEvents = IOComponent->GetEvents();
	for (const FActorIOEvent& IOEvent : ValidEvents)
	{
		SelectableEvents.Emplace(IOEvent.EventName);
	}
}

void SActorIOAction::UpdateSelectableFunctions()
{
	SelectableFunctions.Reset();
	SelectableFunctions.Add(TEXT("<Clear>"));

	TArray<FActorIOFunction> ValidFunctions = IOComponent->GetFunctions();
	for (const FActorIOFunction& IOFunction : ValidFunctions)
	{
		SelectableFunctions.Emplace(IOFunction.FunctionName);
	}
}

FActorIOAction& SActorIOAction::GetAction() const
{
	check(IOComponent && IOComponent->GetActions().IsValidIndex(ActionIdx));
	return IOComponent->GetActions()[ActionIdx];
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
