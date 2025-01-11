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

	const FVector2D Icon16x16(16.0f, 16.0f);

	StyleSet->Set("Action.OutputIcon", new IMAGE_BRUSH_SVG("icon_action_output", Icon16x16, FColor(0, 215, 255).ReinterpretAsLinear()));
	StyleSet->Set("Action.InputIcon", new IMAGE_BRUSH_SVG("icon_action_input", Icon16x16, FColor(255, 25, 0).ReinterpretAsLinear()));
	StyleSet->Set("Action.TargetIcon", new IMAGE_BRUSH_SVG("icon_action_target", Icon16x16));

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
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

const FName& FActorIOEditorStyle::GetStyleSetName()
{
	static const FName StyleSetName(TEXT("ActorIOEditorStyle"));
	return StyleSetName;
}

#undef RootToContentDir
#undef RootToCoreContentDir
