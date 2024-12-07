// Copyright 2024 Horizon Games. All Rights Reserved.

#include "SActorIOActionList.h"
#include "SActorIOAction.h"
#include "ActorIOSystem.h"
#include "ActorIOComponent.h"
#include "ActorIOAction.h"
#include "ActorIOEditorSubsystem.h"
#include "ActorIOEditorStyle.h"

#define LOCTEXT_NAMESPACE "ActorIOEditor"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

//=======================================================
//~ Begin SActorIOActionList
//=======================================================

SLATE_IMPLEMENT_WIDGET(SActorIOActionList)
void SActorIOActionList::PrivateRegisterAttributes(FSlateAttributeInitializer& AttributeInitializer)
{
}

void SActorIOActionList::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
        SNew(SBorder)
        .BorderImage(FActorIOEditorStyle::Get().GetBrush("ActionList.Body"))
        .Padding(0.0f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(1.0f, 1.0f, 1.0f, 0.0f)
            [
                SNew(SBorder)
                .BorderImage(FActorIOEditorStyle::Get().GetBrush("ActionList.Header"))
                .Padding(0.0f)
                [
                    SAssignNew(PropertyHeaderContainer, SBox)
                    .HeightOverride(FActorIOEditorStyle::HeaderRowHeight)
                    [
                        SAssignNew(PropertyHeaderSplitter, SSplitter)
                        .PhysicalSplitterHandleSize(1.0f)
                    ]
                ]
            ]
            + SVerticalBox::Slot()
            [
                SNew(SScrollBox)
                .ScrollBarThickness(FVector2D(FActorIOEditorStyle::ActionListScrollbarThickness))
                .OnScrollBarVisibilityChanged(this, &SActorIOActionList::OnScrollBarVisibilityChanged)
                + SScrollBox::Slot()
                [
                    SAssignNew(ActionList, SVerticalBox)
                ]
            ]
        ]
	];

    InitializeHeaderRow();

    if (PropertyHeaderSizes.IsEmpty())
    {
        const int32 NumSlots = PropertyHeaderSplitter->GetChildren()->Num();
        PropertyHeaderSizes.Init(1.0f, NumSlots);

        for (int32 SlotIdx = 0; SlotIdx != NumSlots; ++SlotIdx)
        {
            const float TargetSize = PropertyHeaderSplitter->SlotAt(SlotIdx).GetSizeValue();
            PropertyHeaderSizes[SlotIdx] = TargetSize;
        }
    }

    for (int32 SlotIdx = 0; SlotIdx != PropertyHeaderSizes.Num(); ++SlotIdx)
    {
        OnPropertyHeaderResized(SlotIdx, PropertyHeaderSizes[SlotIdx]);
    }
}

void SActorIOActionList::AddPropertyHeader(const FText& InPropertyName, float InSizeValue, const FMargin& InPadding)
{
    const int32 PropertyIndex = PropertyHeaderSplitter->GetChildren()->Num();

    PropertyHeaderSplitter->AddSlot()
    .Value(InSizeValue)
    .OnSlotResized_Lambda([this, PropertyIndex](float InSize) { OnPropertyHeaderResized(PropertyIndex, InSize); })
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

void SActorIOActionList::OnPropertyHeaderResized(int32 InSlotIndex, float InSize)
{
    PropertyHeaderSizes[InSlotIndex] = InSize;
    PropertyHeaderSplitter->SlotAt(InSlotIndex).SetSizeValue(InSize);

    if (FChildren* ChildWidgets = ActionList->GetChildren())
    {
        ChildWidgets->ForEachWidget([InSlotIndex, InSize](SWidget& ChildWidget)
        {
            const FName WidgetType = ChildWidget.GetType();
            if (WidgetType == TEXT("SActorOutputAction") || WidgetType == TEXT("SActorInputAction"))
            {
                SActorIOAction& ActionWidget = static_cast<SActorIOAction&>(ChildWidget);
                ActionWidget.SetPropertySize(InSlotIndex, InSize);
            }
        });
    }
}

void SActorIOActionList::OnScrollBarVisibilityChanged(EVisibility InVisibility)
{
    FMargin Padding = FMargin(0.0f);
    if (InVisibility == EVisibility::Visible)
    {
        Padding.Right = FActorIOEditorStyle::ActionListScrollbarThickness;
    }

    PropertyHeaderContainer->SetPadding(Padding);
}


//=======================================================
//~ Begin SActorOutputList
//=======================================================

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
                .PropertySizes(PropertyHeaderSizes)
            ];
        }

        // Add closing gap between last action and bottom of scroll box.
        if (ActorIOComponent->GetNumActions() > 0)
        {
            ActionList->AddSlot()
            .AutoHeight()
            [
                SNew(SSpacer)
                .Size(FVector2D(0.0f, FActorIOEditorStyle::ActionSpacing))
            ];
        }
    }
}


//=======================================================
//~ Begin SActorInputList
//=======================================================

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
            .PropertySizes(PropertyHeaderSizes)
        ];
    }
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
