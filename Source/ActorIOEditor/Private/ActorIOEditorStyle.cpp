// Copyright 2022-2024 Horizon Games. All Rights Reserved.

#include "ActorIOEditorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateTypes.h"
#include "Interfaces/IPluginManager.h"
#include "Brushes/SlateRoundedBoxBrush.h"

#define RootToContentDir StyleSet->RootToContentDir
#define RootToCoreContentDir StyleSet->RootToCoreContentDir

TUniquePtr<FSlateStyleSet> FActorIOEditorStyle::StyleSet;

void FActorIOEditorStyle::Initialize()
{
	if (StyleSet.IsValid())
	{
		return;
	}

	const FString PluginResources = IPluginManager::Get().FindPlugin(TEXT("ActorIO"))->GetBaseDir() / TEXT("Resources");
	const FString EditorResources = FPaths::EngineContentDir() / TEXT("Slate");

	StyleSet = MakeUnique<FSlateStyleSet>(GetStyleSetName());
	StyleSet->SetContentRoot(PluginResources);
	StyleSet->SetCoreContentRoot(EditorResources);

	StyleSet->Set("RoundedHeader", new FSlateRoundedBoxBrush(FStyleColors::Header, FStyleColors::Input, 1.0f));

	const FCheckBoxStyle ToggleButtonStyle = FCheckBoxStyle()
		.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
		.SetCheckedImage(FSlateRoundedBoxBrush(FStyleColors::Primary, 4.0f, FStyleColors::Input, 1.0f))
		.SetCheckedHoveredImage(FSlateRoundedBoxBrush(FStyleColors::PrimaryHover, 4.0f, FStyleColors::Input, 1.0f))
		.SetCheckedPressedImage(FSlateRoundedBoxBrush(FStyleColors::Dropdown, 4.0f, FStyleColors::Input, 1.0f))
		.SetUncheckedImage(FSlateRoundedBoxBrush(FStyleColors::Secondary, 4.0f, FStyleColors::Input, 1.0f))
		.SetUncheckedHoveredImage(FSlateRoundedBoxBrush(FStyleColors::Hover, 4.0f, FStyleColors::Input, 1.0f))
		.SetUncheckedPressedImage(FSlateRoundedBoxBrush(FStyleColors::Header, 4.0f, FStyleColors::Input, 1.0f))
		.SetForegroundColor(FStyleColors::ForegroundHover)
		.SetHoveredForegroundColor(FStyleColors::ForegroundHover)
		.SetPressedForegroundColor(FStyleColors::ForegroundHover)
		.SetCheckedForegroundColor(FStyleColors::ForegroundHover)
		.SetCheckedHoveredForegroundColor(FStyleColors::ForegroundHover)
		.SetCheckedPressedForegroundColor(FStyleColors::ForegroundHover)
		.SetPadding(0.0f);

	StyleSet->Set("ToggleButtonCheckbox", ToggleButtonStyle);
	StyleSet->Set("ToggleButtonHeight", 38.0f);

	const FButtonStyle ImageButtonStyle = FButtonStyle()
		.SetNormal(FSlateRoundedBoxBrush(FStyleColors::Secondary, 4.0f, FStyleColors::Input, 1.0f))
		.SetHovered(FSlateRoundedBoxBrush(FStyleColors::Hover, 4.0f, FStyleColors::Input, 1.0f))
		.SetPressed(FSlateRoundedBoxBrush(FStyleColors::Header, 4.0f, FStyleColors::Input, 1.0f))
		.SetDisabled(FSlateRoundedBoxBrush(FStyleColors::Dropdown, 4.0f, FStyleColors::Recessed, 1.0f))
		.SetNormalForeground(FStyleColors::ForegroundHover)
		.SetHoveredForeground(FStyleColors::ForegroundHover)
		.SetPressedForeground(FStyleColors::ForegroundHover)
		.SetDisabledForeground(FStyleColors::Foreground)
		.SetNormalPadding(FMargin(0.0f))
		.SetPressedPadding(FMargin(0.0f, 2.5f, 0.0f, 0.5f));

	StyleSet->Set("ImageButton", ImageButtonStyle);

	const FTableViewStyle ActionListStyle = FTableViewStyle()
		.SetBackgroundBrush(FSlateNoResource());

	StyleSet->Set("ActionListView", ActionListStyle);
	StyleSet->Set("ActionListView.Border", new FSlateRoundedBoxBrush(FStyleColors::Recessed, FVector4(0.0f, 0.0f, 4.0f, 4.0f), FStyleColors::Input, 1.0f));
	StyleSet->Set("ActionListView.HeaderRowHeight", 28.0f);
	StyleSet->Set("ActionListView.ActionHeight", 26.0f);
	StyleSet->Set("ActionListView.ActionSpacing", 2.0f);

	SetupActionListColumnSizes();

	//StyleSet->Set("ParamsViewer.Border", new FSlateRoundedBoxBrush(FStyleColors::Dropdown, 3.0f, FStyleColors::Input, 1.0f));
	StyleSet->Set("ParamsViewer.Highlight", new FSlateRoundedBoxBrush(FStyleColors::SelectInactive, 1.0f));

	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon64x64(64.0f, 64.0f);

	StyleSet->Set("Action.OutputIcon", new IMAGE_BRUSH_SVG("Icons/Editor/OutputAction", Icon16x16, FColor(0, 215, 255).ReinterpretAsLinear()));
	StyleSet->Set("Action.InputIcon", new IMAGE_BRUSH_SVG("Icons/Editor/InputAction", Icon16x16, FColor(255, 25, 0).ReinterpretAsLinear()));
	StyleSet->Set("Action.TargetIcon", new IMAGE_BRUSH_SVG("Icons/Editor/ActionTargetArrow", Icon16x16));

	StyleSet->Set("ClassIcon.LogicActorBase", new IMAGE_BRUSH("Icons/AssetIcons/LogicActorBase_16", Icon16x16));
	StyleSet->Set("ClassThumbnail.LogicActorBase", new IMAGE_BRUSH("Icons/AssetIcons/LogicActorBase_64", Icon64x64));
	StyleSet->Set("ClassIcon.LogicBranch", new IMAGE_BRUSH("Icons/AssetIcons/LogicBranch_16", Icon16x16));
	StyleSet->Set("ClassThumbnail.LogicBranch", new IMAGE_BRUSH("Icons/AssetIcons/LogicBranch_64", Icon64x64));
	StyleSet->Set("ClassIcon.LogicCase", new IMAGE_BRUSH("Icons/AssetIcons/LogicCase_16", Icon16x16));
	StyleSet->Set("ClassThumbnail.LogicCase", new IMAGE_BRUSH("Icons/AssetIcons/LogicCase_64", Icon64x64));
	StyleSet->Set("ClassIcon.LogicCompare", new IMAGE_BRUSH("Icons/AssetIcons/LogicCompare_16", Icon16x16));
	StyleSet->Set("ClassThumbnail.LogicCompare", new IMAGE_BRUSH("Icons/AssetIcons/LogicCompare_64", Icon64x64));
	StyleSet->Set("ClassIcon.LogicCounter", new IMAGE_BRUSH("Icons/AssetIcons/LogicCounter_16", Icon16x16));
	StyleSet->Set("ClassThumbnail.LogicCounter", new IMAGE_BRUSH("Icons/AssetIcons/LogicCounter_64", Icon64x64));
	StyleSet->Set("ClassIcon.LogicGlobalEvent", new IMAGE_BRUSH("Icons/AssetIcons/LogicGlobalEvent_16", Icon16x16));
	StyleSet->Set("ClassThumbnail.LogicGlobalEvent", new IMAGE_BRUSH("Icons/AssetIcons/LogicGlobalEvent_64", Icon64x64));
	StyleSet->Set("ClassIcon.LogicRelay", new IMAGE_BRUSH("Icons/AssetIcons/LogicRelay_16", Icon16x16));
	StyleSet->Set("ClassThumbnail.LogicRelay", new IMAGE_BRUSH("Icons/AssetIcons/LogicRelay_64", Icon64x64));
	StyleSet->Set("ClassIcon.LogicSpawner", new IMAGE_BRUSH("Icons/AssetIcons/LogicSpawner_16", Icon16x16));
	StyleSet->Set("ClassThumbnail.LogicSpawner", new IMAGE_BRUSH("Icons/AssetIcons/LogicSpawner_64", Icon64x64));
	StyleSet->Set("ClassIcon.LogicTimeline", new IMAGE_BRUSH("Icons/AssetIcons/LogicTimeline_16", Icon16x16));
	StyleSet->Set("ClassThumbnail.LogicTimeline", new IMAGE_BRUSH("Icons/AssetIcons/LogicTimeline_64", Icon64x64));
	StyleSet->Set("ClassIcon.LogicTimer", new IMAGE_BRUSH("Icons/AssetIcons/LogicTimer_16", Icon16x16));
	StyleSet->Set("ClassThumbnail.LogicTimer", new IMAGE_BRUSH("Icons/AssetIcons/LogicTimer_64", Icon64x64));

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
}

void FActorIOEditorStyle::SetupActionListColumnSizes()
{
	// Size values for action list columns.
	// There must be an entry for each resizable SActorIOActionList::ColumnId!
	// Using two sets to support different sizes for inputs/outputs.

	StyleSet->Set("ActionListView.InputColumnWidth.Caller", 1.0f); // only visible for inputs
	StyleSet->Set("ActionListView.InputColumnWidth.Event", 1.0f);
	StyleSet->Set("ActionListView.InputColumnWidth.Target", 1.0f);
	StyleSet->Set("ActionListView.InputColumnWidth.Action", 1.0f);
	StyleSet->Set("ActionListView.InputColumnWidth.Parameter", 1.0f);
	StyleSet->Set("ActionListView.InputColumnWidth.Delay", 0.35f);
	StyleSet->Set("ActionListView.InputColumnWidth.OnlyOnce", 0.5f);

	StyleSet->Set("ActionListView.OutputColumnWidth.Caller", 1.0f);
	StyleSet->Set("ActionListView.OutputColumnWidth.Event", 1.0f);
	StyleSet->Set("ActionListView.OutputColumnWidth.Target", 1.0f); // only visible for outputs
	StyleSet->Set("ActionListView.OutputColumnWidth.Action", 1.0f);
	StyleSet->Set("ActionListView.OutputColumnWidth.Parameter", 1.0f);
	StyleSet->Set("ActionListView.OutputColumnWidth.Delay", 0.35f);
	StyleSet->Set("ActionListView.OutputColumnWidth.OnlyOnce", 0.5f);
}

void FActorIOEditorStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		StyleSet.Reset();
	}
}

const ISlateStyle& FActorIOEditorStyle::Get()
{
	return *(StyleSet.Get());
}

FSlateStyleSet* FActorIOEditorStyle::GetMutableStyle()
{
	if (StyleSet.IsValid())
	{
		return StyleSet.Get();
	}

	return nullptr;
}

const FName& FActorIOEditorStyle::GetStyleSetName()
{
	static const FName StyleSetName(TEXT("ActorIOEditorStyle"));
	return StyleSetName;
}

#undef RootToContentDir
#undef RootToCoreContentDir
