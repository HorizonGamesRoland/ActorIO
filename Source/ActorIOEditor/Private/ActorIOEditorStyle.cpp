// Copyright 2022-2024 Horizon Games. All Rights Reserved.

#include "ActorIOEditorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr<FSlateStyleSet> FActorIOEditorStyle::Style = nullptr;

void FActorIOEditorStyle::Initialize()
{
	if (Style.IsValid())
	{
		return;
	}

	const FVector2D Icon16x16 = FVector2D(16.0f, 16.0f);

	const FString PluginResources = IPluginManager::Get().FindPlugin(TEXT("ActorIO"))->GetBaseDir() / TEXT("Resources");
	const FString EditorResources = FPaths::EngineContentDir() / TEXT("Editor/Slate");

	Style = MakeShareable(new FSlateStyleSet("ActorIOEditorStyle"));
	Style->SetContentRoot(PluginResources);
	Style->SetCoreContentRoot(EditorResources);
	
	//Style->Set("ActorIO.TabIcon", new IMAGE_BRUSH_SVG("Starship/Common/Event", Icon16x16));

	FSlateStyleRegistry::RegisterSlateStyle(*Style.Get());
}

void FActorIOEditorStyle::Shutdown()
{
	if (Style.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*Style.Get());
		Style.Reset();
	}
}

const ISlateStyle& FActorIOEditorStyle::Get()
{
	return *(Style.Get());
}

const FName& FActorIOEditorStyle::GetStyleSetName()
{
	return Style->GetStyleSetName();
}
