// Copyright 2024 Horizon Games. All Rights Reserved.

#include "SActorIOActionList.h"
#include "SActorIOAction.h"
#include "ActorIOSystem.h"
#include "ActorIOComponent.h"
#include "ActorIOAction.h"
#include "ActorIOEditorSubsystem.h"

#define LOCTEXT_NAMESPACE "ActorIOEditor"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

SLATE_IMPLEMENT_WIDGET(SActorIOActionList)
void SActorIOActionList::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SActorIOActionList::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::Get().GetBrush("Brushes.Header"))
            [
                SNew(SBox)
                .HeightOverride(25.0f)
                [
                    SAssignNew(ActionPropertySplitter, SSplitter)
                    .PhysicalSplitterHandleSize(1.0f)
                ]
            ]
        ]
        + SVerticalBox::Slot()
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::Get().GetBrush("Brushes.Recessed"))
            [
                SAssignNew(ActionList, SVerticalBox)
            ]
        ]
	];

    InitializeHeaderRow();

    if (ActionPropertySizes.IsEmpty())
    {
        const int32 NumSlots = ActionPropertySplitter->GetChildren()->Num();
        ActionPropertySizes.Init(1.0f, NumSlots);

        for (int32 SlotIdx = 0; SlotIdx != NumSlots; ++SlotIdx)
        {
            const float TargetSize = ActionPropertySplitter->SlotAt(SlotIdx).GetSizeValue();
            ActionPropertySizes[SlotIdx] = TargetSize;
        }
    }

    for (int32 SlotIdx = 0; SlotIdx != ActionPropertySizes.Num(); ++SlotIdx)
    {
        OnActionPropertyResized(SlotIdx, ActionPropertySizes[SlotIdx]);
    }
}

void SActorIOActionList::AddPropertyHeader(const FText& InPropertyName, float InSizeValue, const FMargin& InPadding)
{
    const int32 PropertyIndex = ActionPropertySplitter->GetChildren()->Num();

    ActionPropertySplitter->AddSlot()
    .Value(InSizeValue)
    .OnSlotResized_Lambda([this, PropertyIndex](float InSize) { OnActionPropertyResized(PropertyIndex, InSize); })
    [
        SNew(SBox)
        .VAlign(VAlign_Center)
        .Padding(InPadding)
        [
            SNew(STextBlock)
            .Text(InPropertyName)
        ]
    ];
}

void SActorIOActionList::OnActionPropertyResized(int32 InSlotIndex, float InSize)
{
    ActionPropertySizes[InSlotIndex] = InSize;
    ActionPropertySplitter->SlotAt(InSlotIndex).SetSizeValue(InSize);

    if (FChildren* ChildWidgets = ActionList->GetChildren())
    {
        ChildWidgets->ForEachWidget([InSlotIndex, InSize](SWidget& ChildWidget)
        {
            SActorIOAction& ActionWidget = static_cast<SActorIOAction&>(ChildWidget);
            ActionWidget.SetPropertySize(InSlotIndex, InSize);
        });
    }
}


SLATE_IMPLEMENT_WIDGET(SActorOutputList)
void SActorOutputList::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SActorOutputList::Construct(const FArguments& InArgs)
{
    Super::Construct(SActorIOActionList::FArguments());
}

void SActorOutputList::InitializeHeaderRow()
{
    AddPropertyHeader(LOCTEXT("Event", "Event:"), 1.0f, FMargin(30.0f, 0.0f, 0.0f, 0.0f));
    AddPropertyHeader(LOCTEXT("Target", "Target:"), 1.0f, FMargin(5.0f, 0.0f, 0.0f, 0.0f));
    AddPropertyHeader(LOCTEXT("Action", "Action:"), 1.0f, FMargin(5.0f, 0.0f, 0.0f, 0.0f));
    AddPropertyHeader(LOCTEXT("Parameter", "Parameter:"), 1.0f, FMargin(5.0f, 0.0f, 0.0f, 0.0f));
    AddPropertyHeader(LOCTEXT("Delay", "Delay:"), 0.35f, FMargin(5.0f, 0.0f, 0.0f, 0.0f));
    AddPropertyHeader(LOCTEXT("Once", "Once?"), 0.5f, FMargin(5.0f, 0.0f, 0.0f, 0.0f));
}

void SActorOutputList::Refresh()
{
    ActionList->ClearChildren();

    UActorIOEditorSubsystem* ActorIOEditorSubsystem = GEditor->GetEditorSubsystem<UActorIOEditorSubsystem>();
    AActor* SelectedActor = ActorIOEditorSubsystem ? ActorIOEditorSubsystem->GetSelectedActor() : nullptr;
    UActorIOComponent* ActorIOComponent = SelectedActor ? SelectedActor->GetComponentByClass<UActorIOComponent>() : nullptr;

    if (ActorIOComponent)
    {
        ActorIOComponent->RemoveInvalidActions();
        for (const TWeakObjectPtr<UActorIOAction>& Action : ActorIOComponent->GetActions())
        {
            ActionList->AddSlot()
            .AutoHeight()
            [
                SNew(SActorOutputAction)
                .Action(Action)
                .PropertySizes(ActionPropertySizes)
            ];
        }
    }
}


SLATE_IMPLEMENT_WIDGET(SActorInputList)
void SActorInputList::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SActorInputList::Construct(const FArguments& InArgs)
{
    Super::Construct(SActorIOActionList::FArguments());
}

void SActorInputList::InitializeHeaderRow()
{
    AddPropertyHeader(LOCTEXT("Caller", "Caller:"), 1.0f, FMargin(30.0f, 0.0f, 0.0f, 0.0f));
    AddPropertyHeader(LOCTEXT("Event", "Target:"), 1.0f, FMargin(5.0f, 0.0f, 0.0f, 0.0f));
    AddPropertyHeader(LOCTEXT("Action", "Action:"), 1.0f, FMargin(5.0f, 0.0f, 0.0f, 0.0f));
    AddPropertyHeader(LOCTEXT("Parameter", "Parameter:"), 1.0f, FMargin(5.0f, 0.0f, 0.0f, 0.0f));
    //AddPropertyHeader(LOCTEXT("Delay", "Delay:"), FMargin(5.0f, 0.0f));
    //AddPropertyHeader(LOCTEXT("Once", "Once?"), FMargin(5.0f, 0.0f));
}

void SActorInputList::Refresh()
{
    ActionList->ClearChildren();

    UActorIOEditorSubsystem* ActorIOEditorSubsystem = GEditor->GetEditorSubsystem<UActorIOEditorSubsystem>();
    AActor* SelectedActor = ActorIOEditorSubsystem ? ActorIOEditorSubsystem->GetSelectedActor() : nullptr;

    for (const TWeakObjectPtr<UActorIOAction>& InputAction : UActorIOSystem::GetInputActionsForObject(SelectedActor))
    {
        ActionList->AddSlot()
        .AutoHeight()
        [
            SNew(SActorInputAction)
            .Action(InputAction)
            .PropertySizes(ActionPropertySizes)
        ];
    }
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
