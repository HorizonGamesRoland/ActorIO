// Copyright 2022-2024 Horizon Games. All Rights Reserved.

#include "ActorIOEditorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateStyleMacros.h"
#include "Styling/SlateTypes.h"
#include "Interfaces/IPluginManager.h"
#include "Brushes/SlateRoundedBoxBrush.h"

#define RootToContentDir StyleSet->RootToContentDir

TUniquePtr<FSlateStyleSet> FActorIOEditorStyle::StyleSet;

void FActorIOEditorStyle::Initialize()
{
	if (StyleSet.IsValid())
	{
		return;
	}

	const FString PluginResources = IPluginManager::Get().FindPlugin(TEXT("ActorIO"))->GetBaseDir() / TEXT("Resources");
	const FString EditorResources = FPaths::EngineContentDir() / TEXT("Editor/Slate");

	StyleSet = MakeUnique<FSlateStyleSet>(GetStyleSetName());
	StyleSet->SetContentRoot(PluginResources);
	StyleSet->SetCoreContentRoot(EditorResources);

	StyleSet->Set("RoundedHeaderBrush", new FSlateRoundedBoxBrush(FStyleColors::Header, FStyleColors::Black, 1.0f));
	StyleSet->Set("ActionList.Header", new FSlateRoundedBoxBrush(FStyleColors::Header, 4.0f, FStyleColors::Black, 0.0f));
	StyleSet->Set("ActionList.Body", new FSlateRoundedBoxBrush(FStyleColors::Recessed, 4.0f, FStyleColors::Black, 1.0f));

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
